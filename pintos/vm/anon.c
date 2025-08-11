/* anon.c: Implementation of page for non-disk image (a.k.a. anonymous page). */

#include "devices/disk.h"
#include "vm/vm.h"

//[08.11] 헤더 , 전역변수 추가
#include "bitmap.h"
struct bitmap* global_bitmap;
struct disk* swap_disk;

/* DO NOT MODIFY BELOW LINE */
static struct disk *swap_disk;
static bool anon_swap_in(struct page *page, void *kva);
static bool anon_swap_out(struct page *page);
static void anon_destroy(struct page *page);

/* DO NOT MODIFY this struct */
static const struct page_operations anon_ops = {
    .swap_in = anon_swap_in,
    .swap_out = anon_swap_out,
    .destroy = anon_destroy,
    .type = VM_ANON,
};

/* Initialize the data for anonymous pages */
void vm_anon_init(void) {
   
    // swap 파티션의 주소 반환
    swap_disk= disk_get(1,1);

    // swap 은 page 단위로 이루어지므로
    // [섹터개수 / 8] 의 bitmap 필요
    global_bitmap = bitmap_create(disk_size(swap_disk) /8);

}

/* Initialize the file mapping */
bool anon_initializer(struct page *page, enum vm_type type, void *kva) {
    /* Set up the handler */
    page->operations = &anon_ops;

    struct anon_page *anon_page = &page->anon;

    // 08.05 : 단계적 구현을 위해 임시로 return true 
    return true;
}

/* Swap in the page by read contents from the swap disk. */
static bool anon_swap_in(struct page *page, void *kva) {
    
    struct anon_page *anon_page = &page->anon;
    int idx=anon_page->swap_idx;; //초기화
  
    // 디스크에서 섹터 단위(512 byte) 로 읽어서 RAM에 쓰기
    for (int i=0; i<8 ; i++){
    disk_read(swap_disk, idx *8 + i  , kva + i * 512);
    }

    // 비트 1->0 변경 
    bitmap_set(global_bitmap,idx,0);

    return true;
}

/* Swap out the page by writing contents to the swap disk. */
static bool anon_swap_out(struct page *page) {

    size_t start = 0; // bitmap_scan 에서 활용
    int idx =-1; //  초기화 

    struct anon_page *anon_page = &page->anon;

    //1. First-fit 으로 인덱스 찾기
    // bitmap_scan -> scan_and_flip 으로 수정
    // 비트 0 ->1 변경
    idx =bitmap_scan_and_flip(global_bitmap,start,1,false);

    if (idx == BITMAP_ERROR){
        PANIC("Error!\n"); // 빈자리 못찾음
    }

    // 2. 페이지 단위로 디스크에 쓰기 : [1 Page = 8 Sector]
    // RAM의 kva로부터 512씩 8번 읽어서, swap_disk에 쓰기
    for (int i=0; i<8 ; i++){
    disk_write(swap_disk, idx *8 + i  , (page->frame->kva) + i * 512);
    }
    // 3. swap slot 인덱스 저장
    anon_page->swap_idx = idx;

    // 4. 페이지테이블 매핑 해제 -> vm_evict_frame에서 수행??
    return true;
}

/* Destroy the anonymous page. PAGE will be freed by the caller. */
static void anon_destroy(struct page *page) {
    struct anon_page *anon_page = &page->anon;
}
