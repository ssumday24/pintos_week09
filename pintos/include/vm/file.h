#ifndef VM_FILE_H
#define VM_FILE_H
#include "filesys/file.h"
#include "vm/vm.h"

struct page;
enum vm_type;

// [08.12] 구조체 내용 추가 
// 페이지폴트가 발생했을때 필요한 정보
struct file_page {
    bool is_file;                  // 파일의 존재여부
    struct file *file;          // 파일 주소
    off_t ofs;                  // 파일 내 읽을 위치
    size_t page_read_bytes;     // 읽을 바이트 수
    size_t page_zero_bytes;     // zero 패딩할 바이트 수
};

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
