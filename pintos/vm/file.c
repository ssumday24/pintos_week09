/* file.c: Implementation of memory backed file object (mmaped object). */

#include "vm/vm.h"
#include "vm/file.h"
#include "userprog/process.h"
#include "threads/vaddr.h"
#include "threads/mmu.h"
#include "threads/malloc.h"


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
    off_t _ofs = ofs;

    // 반복해서 페이지할당 요청
    while (read_bytes > 0) {
        size_t page_read_bytes = read_bytes < PGSIZE ? read_bytes : PGSIZE;
        size_t page_zero_bytes = PGSIZE - page_read_bytes;

        struct file_aux *aux = malloc(sizeof(struct file_aux));
        aux -> file = file;
        aux -> ofs = _ofs;
        aux -> page_read_bytes = page_read_bytes;
        aux -> page_zero_bytes = page_zero_bytes;

        if (!vm_alloc_page_with_initializer(VM_FILE, upage, writable, lazy_load_segment, aux)){
            free(aux);
            return NULL;
        }
        
        read_bytes -= page_read_bytes;
        upage += PGSIZE;
        _ofs += page_read_bytes;
    } 

    // mmap_file 구조체 만들고 삽입
    struct mmap_file *mm = malloc(sizeof(struct mmap_file));
    mm -> addr = addr;
    mm -> file = file;
    mm -> ofs = ofs;
    mm -> length = length;

    list_push_back(&thread_current() -> mmap_list, &mm -> elem);

    // 리스트 삽입 나중에
    return addr;
}

// 특정 mmap_file에 대해 unmap 수행
void munmap_file(struct mmap_file *mf){
    // 페이지 단위 반복
    struct thread *cur = thread_current();
    size_t length = mf -> length;
    size_t write_bytes = file_length(mf -> file);
    void *upage = mf -> addr;
    off_t ofs = mf -> ofs;
    struct page *c_page;

    while (length > 0){
        
        size_t page_write_bytes = write_bytes < PGSIZE ? write_bytes : PGSIZE;

        size_t result;
        // 더티 비트가 1인 경우, 파일에 다시 쓰기
        if (pml4_is_dirty(cur -> pml4, upage)){
            result = file_write_at(mf -> file, upage, page_write_bytes, ofs);
            if (result != page_write_bytes){
                return;
            };
        }

        c_page = spt_find_page(&cur -> spt, upage);
        if (c_page == NULL){
            return;
        }

        // struct frame 존재하면, free해 줌
        if (c_page -> frame != NULL){
            // 해줄 필요 없었음 double free 발생
            // palloc_free_page(c_page -> frame -> kva);
            free(c_page -> frame);
        }

        if (c_page != NULL){
            // 뭔가 동작상 문제가 있는 듯함 아래 함수가?
            // spt_remove_page(&cur -> spt, c_page);
            hash_delete(&(thread_current() -> spt.pages), &(c_page->hash_elem));
            free(c_page);
        }
        

        length -= PGSIZE;
        write_bytes -= page_write_bytes;
        upage += PGSIZE;
        ofs += page_write_bytes;
    }
}

/* Do the munmap */
void do_munmap(void *addr) {
    // 매핑 정보 검색

    struct thread *cur = thread_current();
    struct list_elem *e;
    bool found = false;
    struct mmap_file *mf;

    for (e = list_begin(&cur->mmap_list); e != list_end(&cur->mmap_list); e = list_next(e)){
        mf = list_entry(e, struct mmap_file, elem);
        if (mf -> addr == addr){
            found = true;
            break;
        }
    }

    // 찾지 못한 경우 실패
    if (!found){
        return;
    }
    // 파일 unmapping 수행
    munmap_file(mf);
    // struct thread의 mmap_list에서 빼기
    list_remove(&mf -> elem);
    // file을 close하고, struct mmap_file을 free
    file_close(mf -> file);
    free(mf);
}
