/* file.c: Implementation of memory backed file object (mmaped object). */

#include "vm/vm.h"
#include "userprog/process.h"
#include "threads/vaddr.h"
#include "mmu.h"

// mmap 된 파일에대한 모든정보 저장
struct mmap_file{
    void *addr;         // 시작 페이지의 주소
    struct file *file;  // 파일 주소
    struct list_elem elem;   // struct thread의 mmap_list 삽입용도
    size_t length;      // 매핑된 영역 크기
    size_t ofs;         // offset
};

static bool file_backed_swap_in(struct page *page, void *kva);
static bool file_backed_swap_out(struct page *page);
static void file_backed_destroy(struct page *page);

/* DO NOT MODIFY this struct */
static const struct page_operations file_ops = {
    .swap_in = file_backed_swap_in,
    .swap_out = file_backed_swap_out,
    .destroy = file_backed_destroy,
    .type = VM_FILE,
};

/* The initializer of file vm */
void vm_file_init(void) {}

/* Initialize the file backed page */
bool file_backed_initializer(struct page *page, enum vm_type type, void *kva) {
    /* Set up the handler */
    page->operations = &file_ops;

    struct file_page *file_page = &page->file;
    return true;
}

/* Swap in the page by read contents from the file. */
static bool file_backed_swap_in(struct page *page, void *kva) {
    struct file_page *file_page UNUSED = &page->file;
}

/* Swap out the page by writeback contents to the file. */
static bool file_backed_swap_out(struct page *page) {
    struct file_page *file_page UNUSED = &page->file;
}

/* Destory the file backed page. PAGE will be freed by the caller. */
static void file_backed_destroy(struct page *page) {
    struct file_page *file_page UNUSED = &page->file;
}

/* Do the mmap */
void *do_mmap(void *addr, size_t length, int writable, struct file *file, off_t ofs) {
    file = file_reopen(file);
    if (file == NULL){
        return NULL;
    }

    size_t read_bytes = file_length(file);
    void *upage = addr;

    // 반복해서 페이지할당 요청
    while (read_bytes > 0) {
        size_t page_read_bytes = read_bytes < PGSIZE ? read_bytes : PGSIZE;
        size_t page_zero_bytes = PGSIZE - page_read_bytes;

        struct file_aux *aux = malloc(sizeof(struct file_aux));
        aux -> file = file;
        aux -> ofs = ofs;
        aux -> page_read_bytes = page_read_bytes;
        aux -> page_zero_bytes = page_zero_bytes;

        if (!vm_alloc_page_with_initializer(VM_FILE, upage, writable, lazy_load_segment, aux)){
            free(aux);
            return NULL;
        }
        
        read_bytes -= page_read_bytes;
        upage += PGSIZE;
        ofs += page_read_bytes;
    } 

    // mmap_file 구조체 만들고 삽입
    struct mmap_file *mm = malloc(sizeof(struct mmap_file));
    mm -> addr = addr;
    mm -> file = file;
    mm -> ofs = ofs;
    mm -> length = length;

    // mmap_list에 추가 -> munmap에서 사용
    list_push_back(&thread_current() -> mmap_list, &mm -> elem);

    // 리스트 삽입 나중에
    return addr;
}

/* Do the munmap */
void do_munmap(void *addr) {
    struct thread *cur = thread_current();
    struct list_elem *e;
    struct mmap_file *pmmap_file=NULL; // mmap_list에서 찾을 mmap_file
    bool found =false; // mmap_file 찾았는지 체크
    struct page * page =NULL;
    size_t write_bytes; //총 읽어야 하는 바이트
    size_t ofs;
    size_t page_write_bytes_each; // 한 페이지에 쓸 바이트

    // 1. 매핑 정보 검색
    for (e=list_begin(&cur->mmap_list);e!=list_end(&cur->mmap_list); e = list_next(e))
    {
        pmmap_file=list_entry(e,struct mmap_file,elem);
        
        // 매핑주소가 동일한 mmap_file 찾았을때
        if (pmmap_file->addr ==addr){ 
            found = true;
            break;
        }
    }

    if (found == false){
        return;
    }

    /* 여기서 file_length(pmmap_file) == (pmmap_file -> length)
        인걸까?
    */ 
    //페이지 단위로 쓰기
   
    write_bytes =file_length(pmmap_file);
    ofs = pmmap_file->ofs;
    
   while (write_bytes> 0) { 
    
    //mmap 되어있는 파일 주소
    struct file * file = pmmap_file->file;

    page = spt_find_page(&cur->spt,addr);

    // SPT 에서 못찾았을때
    if (page ==NULL){
            return;
    }
    
    // 한 페이지에 쓸 bytes = min (write_bytes, PGSIZE)
   page_write_bytes_each = write_bytes < PGSIZE ? write_bytes: PGSIZE;

    // 현재페이지가 메모리에 있고, 더티 비트가 1이면 write-back
    
    /* 
        1. 현재페이지가 메모리에 있다는걸 어떻게 검사할까?
        현재 메모리에 없어도 SPT에 존재할 수 있지 않을까?

        2. unmap 인자인 addr 과  page->va 의 차이? 여기서는 둘다 같은값일까?
    */

    if( pml4_is_dirty(cur->pml4,addr)){
        file_write_at(file,page->va, page_write_bytes_each,ofs);
    }

    // 프레임이 존재하면 메모리 해제
    if (page->frame != NULL){
        free(page->frame);
    }
    // hash_delete로쓰기
    hash_delete(&cur->spt.pages,&page->hash_elem);
    addr +=PGSIZE;
    ofs += page_write_bytes_each;
    write_bytes -=page_write_bytes_each;
    }

}
