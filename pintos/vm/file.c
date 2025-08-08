/* file.c: Implementation of memory backed file object (mmaped object). */

#include "vm/vm.h"
#include "userprog/process.h"
#include "threads/vaddr.h"

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

    list_push_back(&thread_current() -> mmap_list, &mm -> elem);

    // 리스트 삽입 나중에
    return addr;
}

/* Do the munmap */
void do_munmap(void *addr) {}
