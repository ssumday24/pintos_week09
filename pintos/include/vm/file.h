#ifndef VM_FILE_H
#define VM_FILE_H
#include "filesys/file.h"
#include "vm/vm.h"

struct page;
enum vm_type;

struct file_page {};

struct mmap_file{
    void *addr;         // 시작 페이지의 주소
    struct file *file;  // 파일 주소
    struct list_elem elem;   // struct thread의 mmap_list 삽입용도
    size_t length;      // 매핑된 영역 크기
    size_t ofs;         // offset
};


void vm_file_init(void);
bool file_backed_initializer(struct page *page, enum vm_type type, void *kva);
void munmap_file(struct mmap_file *mf);
void *do_mmap(void *addr, size_t length, int writable, struct file *file, off_t offset);
void do_munmap(void *va);
#endif
