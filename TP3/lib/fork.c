// implement fork from user space

#include <inc/string.h>
#include <inc/lib.h>

// PTE_COW marks copy-on-write page table entries.
// It is one of the bits explicitly allocated to user processes (PTE_AVAIL).
#define PTE_COW 0x800

//
// Custom page fault handler - if faulting page is copy-on-write,
// map in our own private writable copy.
//
static void
pgfault(struct UTrapframe *utf)
{
	void *addr = (void *) utf->utf_fault_va;
	uint32_t err = utf->utf_err;
	int r;

	// Check that the faulting access was (1) a write, and (2) to a
	// copy-on-write page.  If not, panic.
	// Hint:
	//   Use the read-only page table mappings at uvpt
	//   (see <inc/memlayout.h>).

	// LAB 4: Your code here.

	if (!(err & FEC_PR))
		panic("pgfault: addres is not mapped");

	if (!(err & FEC_WR))
		panic("pgfault: was not a write");

	pte_t pte = uvpt[PGNUM(addr)];
	if (!(pte & PTE_COW))
		panic("pgfault: was not a copy-on-write");

	// Allocate a new page, map it at a temporary location (PFTEMP),
	// copy the data from the old page to the new page, then move the new
	// page to the old page's address.
	// Hint:
	//   You should make three system calls.

	// LAB 4: Your code here.
	addr = ROUNDDOWN(addr, PGSIZE);
	if ((sys_page_alloc(0, PFTEMP, PTE_W | PTE_U | PTE_P) < 0))
		panic("pgfault: fail to allocate new page");
	memcpy(PFTEMP, addr, PGSIZE);
	if ((sys_page_map(0, PFTEMP, 0, addr, PTE_W | PTE_U | PTE_P) < 0))
		panic("pgfault: fail to map new page");
	if ((sys_page_unmap(0, PFTEMP) < 0))
		panic("pgfault: fail to unmap new page");
}

//
// Map our virtual page pn (address pn*PGSIZE) into the target envid
// at the same virtual address.  If the page is writable or copy-on-write,
// the new mapping must be created copy-on-write, and then our mapping must be
// marked copy-on-write as well.  (Exercise: Why do we need to mark ours
// copy-on-write again if it was already copy-on-write at the beginning of
// this function?)
//
// Returns: 0 on success, < 0 on error.
// It is also OK to panic on error.
//
static int
duppage(envid_t envid, unsigned pn)
{
	void *addr = (void *) (pn * PGSIZE);
	pte_t pte = uvpt[pn];
	int perm = PTE_U | PTE_P;

	// If the page is writable or copy-on-write,
	// the new mapping must be created copy-on-write,
	if ((pte & PTE_W) || (pte & PTE_COW))
		perm |= PTE_COW;
	if ((sys_page_map(0, addr, envid, addr, perm) < 0))
		panic("duppage: fail to map");

	// and then our mapping must be
	// marked copy-on-write as well.
	if (perm & PTE_COW)
		if ((sys_page_map(0, addr, 0, addr, perm) < 0))
			panic("duppage: fail to map");

	return 0;
}


static void
dup_or_share(envid_t dstenv, void *va, int perm)
{
	if ((perm & PTE_W) == PTE_W) {
		if (sys_page_alloc(dstenv, va, PTE_P | PTE_U | PTE_W) < 0)
			panic("dup_or_share: sys_page_alloc");
		if (sys_page_map(dstenv, va, 0, UTEMP, PTE_P | PTE_U | PTE_W) < 0)
			panic("dup_or_share: sys_page_map");

		memmove(UTEMP, va, PGSIZE);

		if (sys_page_unmap(0, UTEMP) < 0)
			panic("dup_or_share: sys_page_unmap");
		return;
	}

	if (sys_page_map(dstenv, va, 0, UTEMP, PTE_P | PTE_U) < 0)
		panic("dup_or_share: sys_page_map");
}


envid_t
fork_v0(void)
{
	envid_t envid;
	uint8_t *addr;
	int r;

	envid = sys_exofork();
	if (envid < 0)
		panic("sys_exofork: %e", envid);
	if (envid == 0) {
		thisenv = &envs[ENVX(sys_getenvid())];
		return 0;
	}

	for (addr = 0; addr < (uint8_t *) UTOP; addr += PGSIZE) {
		if (uvpd[PDX(addr)] & PTE_P) {
			pte_t pte = uvpt[PGNUM(addr)];
			if (pte & PTE_P) {
				dup_or_share(envid, addr, pte);
			}
		}
	}

	if ((r = sys_env_set_status(envid, ENV_RUNNABLE)) < 0)
		panic("sys_env_set_status: %e", r);

	return envid;
}


//
// User-level fork with copy-on-write.
// Set up our page fault handler appropriately.
// Create a child.
// Copy our address space and page fault handler setup to the child.
// Then mark the child as runnable and return.
//
// Returns: child's envid to the parent, 0 to the child, < 0 on error.
// It is also OK to panic on error.
//
// Hint:
//   Use uvpd, uvpt, and duppage.
//   Remember to fix "thisenv" in the child process.
//   Neither user exception stack should ever be marked copy-on-write,
//   so you must allocate a new page for the child's user exception stack.
//

envid_t
fork(void)
{
	// LAB 4: Your code here.
	envid_t envid;
	uint8_t *addr;
	int r;

	set_pgfault_handler(pgfault);

	envid = sys_exofork();
	if (envid < 0)
		panic("sys_exofork: %e", envid);
	if (envid == 0) {
		thisenv = &envs[ENVX(sys_getenvid())];
		return 0;
	}

	for (addr = 0; addr < (uint8_t *) UTOP - PGSIZE; addr += PGSIZE) {
		if (uvpd[PDX(addr)] & PTE_P) {
			pte_t pte = uvpt[PGNUM(addr)];
			if (pte & PTE_P)
				duppage(envid, PGNUM(addr));
		}
	}

	if ((r = sys_page_alloc(
	             envid, (void *) (UTOP - PGSIZE), PTE_W | PTE_U | PTE_P)) < 0)
		panic("fork: sys_page_alloc: %e", r);

	extern void _pgfault_upcall(void);
	if ((r = sys_env_set_pgfault_upcall(envid, _pgfault_upcall)) < 0)
		panic("fork: sys_env_set_pgfault_upcall: %e", r);

	if ((r = sys_env_set_status(envid, ENV_RUNNABLE)) < 0)
		panic("fork: sys_env_set_status: %e", r);

	return envid;
}

// Challenge!
int
sfork(void)
{
	panic("sfork not implemented");
	return -E_INVAL;
}
