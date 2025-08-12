#ifndef VM_ANON_H
#define VM_ANON_H
#include "vm/vm.h"
struct page;
enum vm_type;

struct anon_page {
    int swap_idx; 

    /* 
    swap_idx를 저장해두는 이유: 페이지폴트시, 스왑 아웃된 데이터를 어디서 부터
    읽어와야하는지 알아야하기 때문
    */
};

void vm_anon_init(void);
bool anon_initializer(struct page *page, enum vm_type type, void *kva);

#endif
