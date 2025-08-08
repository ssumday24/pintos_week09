#ifndef USERPROG_PROCESS_H
#define USERPROG_PROCESS_H

#include "threads/thread.h"

tid_t process_create_initd(const char *file_name);
tid_t process_fork(const char *name, struct intr_frame *if_);
int process_exec(void *f_name);
int process_wait(tid_t);
void process_exit(void);
void process_activate(struct thread *next);
// process_exec()에서 사용할 함수 선언
// static void argument_stack(char **argv, int argc, struct intr_frame *if_);
static void argument_stack(char **argv, int argc, struct intr_frame *if_, void *buffer);
/* 7.28 추가 자식 pid 구하기 */
struct thread *get_child_with_pid(tid_t tid);

#ifdef VM
// 파일정보 관리용 구조체 선언
struct file_aux {
    struct file *file;          // 파일 주소
    off_t ofs;                  // 파일 내 읽을 위치
    size_t page_read_bytes;     // 읽을 바이트 수
    size_t page_zero_bytes;     // zero 패딩할 바이트 수
};

bool lazy_load_segment(struct page *page, void *aux);
#endif

#endif /* userprog/process.h */


