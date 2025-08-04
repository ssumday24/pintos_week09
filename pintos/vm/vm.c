/* vm.c: Generic interface for virtual memory objects. */

#include "vm/vm.h"

#include "threads/malloc.h"
#include "vm/inspect.h"

//loader_kern_base 매크로 변수를 사용하기 위한 헤더 파일
#include "threads/loader.h"

//pml4_set_page() 함수를 사용하기 위한 헤더 파일
#include "threads/mmu.h"

/* Initializes the virtual memory subsystem by invoking each subsystem's
 * intialize codes. */
void vm_init(void) {
    vm_anon_init();
    vm_file_init();
#ifdef EFILESYS /* For project 4 */
    pagecache_init();
#endif
    register_inspect_intr();
    /* DO NOT MODIFY UPPER LINES. */
    /* TODO: Your code goes here. */
}

/* Get the type of the page. This function is useful if you want to know the
 * type of the page after it will be initialized.
 * This function is fully implemented now. */
enum vm_type page_get_type(struct page *page) {
    int ty = VM_TYPE(page->operations->type);
    switch (ty) {
        case VM_UNINIT:
            return VM_TYPE(page->uninit.type);
        default:
            return ty;
    }
}

/* Helpers */
static struct frame *vm_get_victim(void);
static bool vm_do_claim_page(struct page *page);
static struct frame *vm_evict_frame(void);

/* Create the pending page object with initializer. If you want to create a
 * page, do not create it directly and make it through this function or
 * `vm_alloc_page`. */
bool vm_alloc_page_with_initializer(enum vm_type type, void *upage, bool writable,
                                    vm_initializer *init, void *aux) {
    ASSERT(VM_TYPE(type) != VM_UNINIT)

    struct supplemental_page_table *spt = &thread_current()->spt;

    /* Check wheter the upage is already occupied or not. */
    if (spt_find_page(spt, upage) == NULL) {
        /* TODO: Create the page, fetch the initialier according to the VM type,
         * TODO: and then create "uninit" page struct by calling uninit_new. You
         * TODO: should modify the field after calling the uninit_new. */

        /* TODO: Insert the page into the spt. */
    }
err:
    return false;
}

/* Find VA from spt and return page. On error, return NULL. */
struct page *spt_find_page(struct supplemental_page_table *spt UNUSED, void *va UNUSED) {
    struct page *page = NULL;
    /* TODO: Fill this function. */

    return page;
}

/* Insert PAGE into spt with validation. */
bool spt_insert_page(struct supplemental_page_table *spt UNUSED, struct page *page UNUSED) {
    int succ = false;
    /* TODO: Fill this function. */

    return succ;
}

void spt_remove_page(struct supplemental_page_table *spt, struct page *page) {
    vm_dealloc_page(page);
    return true;
}

/* Get the struct frame, that will be evicted. */
static struct frame *vm_get_victim(void) {
    struct frame *victim = NULL;
    /* TODO: The policy for eviction is up to you. */

    return victim;
}

/* Evict one page and return the corresponding frame.
 * Return NULL on error.*/
static struct frame *vm_evict_frame(void) {
    struct frame *victim UNUSED = vm_get_victim();
    /* TODO: swap out the victim and return the evicted frame. */

    return NULL;
}

/* palloc() and get frame. If there is no available page, evict the page
 * and return it. This always return valid address. That is, if the user pool
 * memory is full, this function evicts the frame to get the available memory
 * space.*/
static struct frame *vm_get_frame(void) {
    struct frame *frame = NULL;

    // 새로운 페이지 할당 
    void * new_page = palloc_get_page(PAL_USER);

    if(new_page == NULL){
        PANIC("todo\n");
    }

    // 프레임 구조체 할당
    frame = calloc(1,sizeof(struct frame));
    if(frame == NULL){
        PANIC("to do\n");
    }
    // frame 구조체 멤버 변수 초기화
    // 실제 물리 메모리 page 할당
    frame->page = NULL;
    // 물리 메모리 주소 -> 가상 주소로 변환
    frame->kva = new_page + KERN_BASE;    

    ASSERT(frame != NULL);
    ASSERT(frame->page == NULL);    
    /* 실제 프레임의 Page는 매핑되기 전까지 빈 New_page를 만들기만 해두고,kva에 newpage에 대한 주소 정보를 담고있어서 나중에 매핑할 때 할당받은 newpage에 데이터를 넣는 느낌인가? */ 
    
    return frame;
}

/* Growing the stack. */
static void vm_stack_growth(void *addr UNUSED) {}

/* Handle the fault on write_protected page */
static bool vm_handle_wp(struct page *page UNUSED) {}

/* Return true on success */
bool vm_try_handle_fault(struct intr_frame *f UNUSED, void *addr UNUSED, bool user UNUSED,
                         bool write UNUSED, bool not_present UNUSED) {
    struct supplemental_page_table *spt UNUSED = &thread_current()->spt;
    struct page *page = NULL;
    /* TODO: Validate the fault */
    /* TODO: Your code goes here */

    return vm_do_claim_page(page);
}

/* Free the page.
 * DO NOT MODIFY THIS FUNCTION. */
void vm_dealloc_page(struct page *page) {
    destroy(page);
    free(page);
}

/* Claim the page that allocate on VA. */
bool vm_claim_page(void *va UNUSED) {
    struct page *page = NULL;

    /* TODO: Fill this function */
    
    // spt는 구조체로 선언되어 있어서 &(주소 연산자)를 붙여줌.
    // EXPECT: 유저 영역의 va와 spt 정보를 넘겨주면 spt에 해당 주소에 대한 정보가 있으면 페이지를 가져올 것을 기대함.
    page = spt_find_page(&thread_current()->spt,va);

    if(page == NULL){
        PANIC("failed to get page!\n");
    }
    
    return vm_do_claim_page(page);
}

/* Claim the PAGE and set up the mmu. */
static bool vm_do_claim_page(struct page *page) {
    /* TODO : you need to set up the MMU. In other words, add the mapping from the virtual address to the physical address in the page table */
    
    struct frame *frame = vm_get_frame();

    if(frame == NULL){  //vm_get_frame()으로 받아온 frame이 NULL이면 예외처리
        PANIC("Failed to Receive frame\n");
    }
    /* Set links */
    frame->page = page;
    page->frame = frame;

    // FIX: pml4_set_page 안에 vtop() 함수 안에서 아래 과정을 처리하고 있어서 주석처리함.
    //void * kpage = frame->kva - KERN_BASE;  // 프레임의 실제 물리 메모리 주소가 나온다.

    /* is_writable() 함수를 사용하기 위해 pte 정보를 가져온다. pml4_get_page 함수 내에서 pte값 가져오는 방식을 참고함.*/
    uint64_t *pte = pml4e_walk(thread_current()->pml4, (uint64_t)page->va, 0);

    /* TODO: Insert page table entry to map page's VA to frame's PA. */
    // FIX : 처음에는 frame->page를 넘겨줬는데, 유저 영역의 VA 와 커널 영역의 실제 메모리 주소를 매핑하는 것이므로 page->va로 변경 
    if(!pml4_set_page(thread_current()->pml4, page->va, frame->kva, is_writable(pte))){  // true, false를 반환하므로, 실패 시 에러 처리
        PANIC("Failed to Insert page table entry to map page's VA to frame's PA");
    }

    return swap_in(page, frame->kva);
}

/* Initialize new supplemental page table */
void supplemental_page_table_init(struct supplemental_page_table *spt UNUSED) {}

/* Copy supplemental page table from src to dst */
bool supplemental_page_table_copy(struct supplemental_page_table *dst UNUSED,
                                  struct supplemental_page_table *src UNUSED) {}

/* Free the resource hold by the supplemental page table */
void supplemental_page_table_kill(struct supplemental_page_table *spt UNUSED) {
    /* TODO: Destroy all the supplemental_page_table hold by thread and
     * TODO: writeback all the modified contents to the storage. */
}
