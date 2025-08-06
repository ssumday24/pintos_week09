/* vm.c: Generic interface for virtual memory objects. */

#include "vm/vm.h"

#include "threads/malloc.h"
#include "vm/vm.h"
#include "vm/inspect.h"

//loader_kern_base 매크로 변수를 사용하기 위한 헤더 파일
// #include "threads/loader.h"

//pml4_set_page() 함수를 사용하기 위한 헤더 파일
#include "threads/mmu.h"
/* ===== 함수 선언 부분 =====*/
unsigned page_hash(const struct hash_elem *p_, void *aux UNUSED);
bool page_less(const struct hash_elem *a_, 
                const struct hash_elem *b_, void *aux UNUSED);


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
 * This function is fully implementesd now. */
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
// Helper 함수라서 static으로 선언.
static bool vm_do_copy(struct page *parent_page,struct supplemental_page_table * child_spt, enum vm_type type, void* va);

/* Create the pending page object with initializer. If you want to create a
 * page, do not create it directly and make it through this function or
 * `vm_alloc_page`. */
bool vm_alloc_page_with_initializer(enum vm_type type, void *upage, bool writable,
                                    vm_initializer *init, void *aux) {
    ASSERT(VM_TYPE(type) != VM_UNINIT)

    struct supplemental_page_table *spt = &thread_current()->spt;
    
    /* Check wheter the upage is already occupied or not. */
    // 가상 주소가 이미 사용중인지 확인
    if (spt_find_page(spt, upage) == NULL) {
        /* TODO: Create the page, fetch the initialier according to the VM type,
         * TODO: and then create "uninit" page struct by calling uninit_new. You
         * TODO: should modify the field after calling the uninit_new. */

        struct page * new_page = malloc(sizeof(struct page));
        
        if(new_page == NULL) goto err;

        if(VM_TYPE(type) == VM_ANON){
            uninit_new(new_page,upage,init,type,aux,anon_initializer);
        }
        else{
            uninit_new(new_page,upage,init,type,aux,file_backed_initializer);
        }
        
        new_page->writable = writable;
        /* TODO: Insert the page into the spt. */

        if(!spt_insert_page(spt,new_page)){
            free(new_page);
            goto err;
        }
    }
    return true;

err:
    return false;
}

/* Find VA from spt and return page. On error, return NULL. */

struct page *spt_find_page(struct supplemental_page_table *spt, void *va) {

// SPT 에서 VA 와 일치하는 페이지 반환
    struct page *page = NULL;
    /* TODO: Fill this function. */
    // supplement page table에서 가상주소가 va인 struct page 찾기
    struct page p;              // 검색용으로 만든 임시 페이지
    struct hash_elem *e;        // hash_find의 검색결과 저장용
    p.va = va;
    e = hash_find(&(spt -> pages), &(p.hash_elem));
    page = e != NULL ? hash_entry(e, struct page, hash_elem): NULL;
    return page;
}

/* Insert PAGE into spt with validation. */
bool spt_insert_page(struct supplemental_page_table *spt, struct page *page) {
    int succ = false;
    /* TODO: Fill this function. */
    // supplement page table에 struct page(의 hash_elem) 삽입하기
    // hash_insert는 성공 시 NULL, 실패 시 (중복 키 존재) 해당 hash_elem의 주소를 반환

    if (hash_insert(&(spt -> pages), &(page -> hash_elem)) == NULL){
        succ = true;
    }
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

    // 새로운 페이지 할당  -> palloc 은 kva 반환
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
    frame->kva = new_page;    

    ASSERT(frame != NULL);
    ASSERT(frame->page == NULL);    
    /* 실제 프레임의 Page는 매핑되기 전까지 빈 New_page를 만들기만 해두고,kva에 newpage에 대한 주소 정보를 담고있어서 나중에 매핑할 때 할당받은 newpage에 데이터를 넣는 느낌인가? */ 
    
    return frame;
}

/* Growing the stack. */
static void vm_stack_growth(void *addr UNUSED) {}

/* Handle the fault on write_protected page */
static bool vm_handle_wp(struct page *page UNUSED) {}

/* Return true on success. */ 
// 페이지 폴트 핸들러
bool vm_try_handle_fault(struct intr_frame *f , void *addr , bool user ,
                         bool write , bool not_present ) {
    struct supplemental_page_table *spt  = &thread_current()->spt;
    struct page *page = NULL;
    /* TODO: Validate the fault */
    // if(is_kern_addr(addr) || addr == NULL || page = spt_find_page(spt,addr) == NULL) return false
    /* TODO: Your code goes here */
 
    /* ===== Invalid 페이지 폴트인지 검사 ===== */
    // 1. addr 가 NULL 일때
    if( addr == NULL){
        return false;
    }

    // 2. 사용자 모드에서 -> 커널 영역에 접근했을때
    if( user == true && is_kernel_vaddr(addr) ) {
        return false;
    }

    // 3. Read-Only 파일에 쓰기 시도
    if ( write == true && not_present == false ){
        return false;
    }

    // spt 해시테이블에서 addr(va) 와 일치하는 페이지 찾기
    page = spt_find_page(spt, pg_round_down(addr));

    // FIX : 예외처리 추가
    if ( page == NULL){
        return false;
    }

    /* lazy-loading , 스왑 처리 */
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
        printf("failed to get page!\n");
        return false;
    }
    return vm_do_claim_page(page);
}

/* Claim the PAGE and set up the mmu. */
static bool vm_do_claim_page(struct page *page) {
    /* TODO : you need to set up the MMU. In other words, add the mapping from the virtual address to the physical address in the page table */
    
    struct frame *frame = vm_get_frame();

    if(frame == NULL){  //vm_get_frame()으로 받아온 frame이 NULL이면 예외처리
        printf("Failed to Receive frame\n");
        return false;
    }
    /* Set links */
    // Eviction 때 링킹 필요
    frame->page = page;
    page->frame = frame;
 
    // FIX: pml4_set_page 안에 vtop() 함수 안에서 아래 과정을 처리하고 있어서 주석처리함.

    /* TODO: Insert page table entry to map page's VA to frame's PA. */
    // FIX : 처음에는 frame->page를 넘겨줬는데, 유저 영역의 VA 와 커널 영역의 실제 메모리 주소를 매핑하는 것이므로 page->va로 변경 
    // FIX : 인자를 writable 로 넘겨주도록 수정
    if(!pml4_set_page(thread_current()->pml4, page->va, frame->kva, page->writable)){  // true, false를 반환하므로, 실패 시 에러 처리
        printf("Failed to Insert page table entry to map page's VA to frame's PA");
        return false;
    }

    return swap_in(page, frame->kva);
}

/* Initialize new supplemental page table */
void supplemental_page_table_init(struct supplemental_page_table *spt) {
    // supplemental page table에 사용할 hash table을 초기화
    hash_init(&(spt -> pages), page_hash, page_less, NULL);
}

/* Copy supplemental page table from src to dst */
bool supplemental_page_table_copy(struct supplemental_page_table *dst UNUSED, struct supplemental_page_table *src UNUSED) 
{
    
    struct hash_iterator it;
    hash_first(&it, &src->pages);

    while(hash_next(&it)){
        struct page * parent_page = hash_entry(hash_cur(&it),struct page, hash_elem);
        if(!vm_do_copy(parent_page,dst,parent_page->operations->type, parent_page->va)) return false;
    }
    return true;
/* TODO: Iterate through each page in the src's supplemental page table and
   TODO: make a exact copy of the entry in the dst's supplemental page table. 
   TODO: You will need to allocate uninit page and claim them immediately.
*/
}

/* Free the resource hold by the supplemental page table */
void supplemental_page_table_kill(struct supplemental_page_table *spt UNUSED) {
    /* TODO: Destroy all the supplemental_page_table hold by thread and
     * TODO: writeback all the modified contents to the storage. */
    struct hash_iterator it;

    hash_first(&it,&spt->pages);
    while(hash_next(&it)){
        struct page * p = hash_entry(hash_cur(&it),struct page,hash_elem);
        /* dirty 상태라면 디스크에 writeback 을 해야함
        if(pml4_is_dirty(thread_current()->pml4,p->va)){
            swap 과 mmap 구현하면 필요할듯
        }
        */
       vm_dealloc_page(p);
    }
}

/* ===== 해시 함수 추가 부분 08.04 ===== */
unsigned page_hash(const struct hash_elem *p_, void *aux UNUSED){
    const struct page *p = hash_entry(p_,struct page, hash_elem);
    return hash_bytes(&p->va,sizeof (p->va));
}

/* ===== 해시 함수 추가 부분 08.04 ===== */
bool page_less(const struct hash_elem *a_, 
                const struct hash_elem *b_, void *aux UNUSED){
    const struct page *a =hash_entry(a_,struct page,hash_elem);
    const struct page *b =hash_entry(b_,struct page, hash_elem);
                
    return a->va < b-> va;
}

bool vm_do_copy(struct page *parent_page,struct supplemental_page_table * child_spt, enum vm_type type,void* va) {

    if (spt_find_page(child_spt, va) == NULL) {

        struct page * child_page = malloc(sizeof(struct page));
        
        if(child_page == NULL) goto err;

        if(VM_TYPE(type) == VM_UNINIT){ //부모 페이지 상태가 Uninit이면 페이지 type을 새로 구해줌
            type = parent_page->uninit.type;
        }

        if(VM_TYPE(type) == VM_ANON){   // anon은 NULL 해도 됨
            uninit_new(child_page,parent_page->va,NULL,type,NULL,anon_initializer);
        }
        else{   //file-backed는 NULL 하면 안 되는데 일단 NULL로 해두고 나중에 바꾸기
            uninit_new(child_page,parent_page->va,NULL,type,NULL,file_backed_initializer);
        }

        child_page->hash_elem = parent_page->hash_elem;
        child_page->writable = parent_page->writable;

        if(!spt_insert_page(child_spt,child_page)){
            free(child_page);
            goto err;
        }
        
        if(!vm_claim_page(child_page->va)){
            free(child_page);
            goto err;
        }
        if(parent_page->frame){
            memcpy(child_page->frame->kva,parent_page->frame->kva,PGSIZE);
        }
    }

    return true;
err:
    return false;
}