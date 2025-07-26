#include "userprog/syscall.h"

#include <stdio.h>
#include <syscall-nr.h>

#include "intrinsic.h"
#include "threads/flags.h"
#include "threads/init.h"
#include "threads/interrupt.h"
#include "threads/loader.h"
#include "threads/thread.h"
#include "userprog/gdt.h"

/* ===== 헤더 파일 추가 07.22 =====*/
#include "filesys/file.h"
#include "filesys/filesys.h"
#include "threads/palloc.h"
#include "threads/synch.h"

#define STDIN_FILENO 0
#define STDOUT_FILENO 1
#define FDT_MAX_SIZE 128
/* ===== 함수 선언 추가 07.23 ===== */
static bool check_address(void *addr);
void syscall_entry(void);
void syscall_handler(struct intr_frame *);

/* ======= syscall 함수 선언 ========*/
void halt(void);
void exit(int status);
void exec(const char *cmd_line);
int wait(tid_t pid);
int write(int fd, const void *buffer, unsigned size);
bool create(const char *file, unsigned initial_size);
bool remove(const char *file);
int open(const char *file_name);
int filesize(int fd);
int read(int fd, void *buffer, unsigned size);

/* ======================================*/

/* System call.
 *
 * Previously system call services was handled by the interrupt handler
 * (e.g. int 0x80 in linux). However, in x86-64, the manufacturer supplies
 * efficient path for requesting the system call, the `syscall` instruction.
 *
 * The syscall instruction works by reading the values from the the Model
 * Specific Register (MSR). For the details, see the manual. */

#define MSR_STAR 0xc0000081         /* Segment selector msr */
#define MSR_LSTAR 0xc0000082        /* Long mode SYSCALL target */
#define MSR_SYSCALL_MASK 0xc0000084 /* Mask for the eflags */

void syscall_init(void) {
    write_msr(MSR_STAR, ((uint64_t)SEL_UCSEG - 0x10) << 48 | ((uint64_t)SEL_KCSEG) << 32);
    write_msr(MSR_LSTAR, (uint64_t)syscall_entry);

    /* The interrupt service rountine should not serve any interrupts
     * until the syscall_entry swaps the userland stack to the kernel
     * mode stack. Therefore, we masked the FLAG_FL. */
    write_msr(MSR_SYSCALL_MASK, FLAG_IF | FLAG_TF | FLAG_DF | FLAG_IOPL | FLAG_AC | FLAG_NT);
}

/* ====== 메인 시스템콜 인터페이스  => 커널공간 ===== */
void syscall_handler(struct intr_frame *f UNUSED) {
    // TODO: Your implementation goes here.
    // printf ("system call!\n");  //이부분 Test때는 주석처리

    int syscall_number = f->R.rax;  // 시스템 콜 번호는 rax 레지스터에 저장됨
    switch (syscall_number) {       // rdi -> rsi -> rdx -> r10 .....
        case SYS_HALT:              // case : 0
            halt();
            break;
        case SYS_EXIT:  // case : 1
            exit(f->R.rdi);
            break;
        // case SYS_FORK:  // case : 2
        //     f->R.rax = fork_syscall (f->R.rdi, f);
        //     break;
        case SYS_EXEC:  // case : 3
            exec(f->R.rdi);
            break;
        case SYS_WAIT:  // case : 4
            f->R.rax = wait(f->R.rdi);
            break;
        case SYS_CREATE:  // case : 5
            f->R.rax = create(f->R.rdi, f->R.rsi);
            break;
        case SYS_REMOVE:  // case : 6
            f->R.rax = remove(f->R.rdi);
            break;
        case SYS_OPEN:  // case : 7
            f->R.rax = open(f->R.rdi);
            break;
        case SYS_FILESIZE:  // case : 8
            f->R.rax = filesize(f->R.rdi);
            break;
        case SYS_READ:  // case : 9
            f->R.rax = read(f->R.rdi, f->R.rsi, f->R.rdx);
            break;
        case SYS_WRITE:  // case : 10
            f->R.rax = write(f->R.rdi, f->R.rsi, f->R.rdx);
            break;
        // case SYS_SEEK:
        //     seek (f->R.rdi, f->R.rsi);
        //     break;
        // case SYS_TELL:
        //     f->R.rax = tell (f->R.rdi);
        //     break;
        case SYS_CLOSE:  // case : 13
            close(f->R.rdi);
            break;
        // case SYS_DUP2:
        //     f->R.rax = dup2 (f->R.rdi, f->R.rsi);
        //     break;
        default:
            printf("Unknown system call: %d\n", syscall_number);
            thread_exit();
            break;
    }
}
///////////////////////////////////// 커널 측 시스템콜 함수 구현 ///////////////////////////////////
void halt(void) {  // Case : 0
    power_off();
}

void exit(int status) {  // Case : 1
    /* 1.현재 실행중인 프로세스(나) 를 종료시킨다.
        2. 내가 종료될 때의 상태를 기록해서, 부모 프로세스가 알수 있도록 한다.
    */

    struct thread *cur = thread_current();
    cur->exit_status = status;  // 나의 exit 상태 기록
    printf("%s: exit(%d)\n", cur->name, status);
    thread_exit();
}

// tid_t fork(const char *thread_name) {  // Case : 2
//     exit(-1);
// }

void exec(const char *cmd_line) {
    // 1. 유저가 제공한 포인터가 유효한지 1차적으로 확인
    if (!check_address((void *)cmd_line)) {
        exit(-1);
    }

    // 2. 커널 공간에 cmd_line의 복사본을 만들기 위해 페이지 할당
    char *cmd_line_copy = palloc_get_page(0);
    if (cmd_line_copy == NULL) {
        exit(-1);
    }

    // 3. strlcpy를 사용하는 대신, 한 바이트씩 주소를 확인하며 안전하게 복사
    const char *user_p = cmd_line;
    char *kernel_p = cmd_line_copy;

    while (true) {
        // 읽으려는 유저 주소가 유효한지 매번 확인
        if (!check_address((void *)user_p)) {
            palloc_free_page(cmd_line_copy);
            exit(-1);
        }

        // 데이터 복사
        *kernel_p = *user_p;

        // 문자열의 끝이면 종료
        if (*user_p == '\0') {
            break;
        }

        user_p++;
        kernel_p++;

        // 페이지 크기를 넘어서는 복사를 방지
        if ((size_t)(kernel_p - cmd_line_copy) >= PGSIZE) {
            palloc_free_page(cmd_line_copy);
            exit(-1);
        }
    }

    // 4. 안전하게 복사된 커널 포인터(cmd_line_copy)를 process_exec에 전달
    if (process_exec(cmd_line_copy) == -1) {
        // process_exec은 성공하면 돌아오지 않으므로, 실패 시에만 종료
        exit(-1);
    }
}

int wait(tid_t pid) {  // Case : 4

    // 자식의 종료 status 받아오기
    return process_wait(pid);
}

bool create(const char *file, unsigned initial_size) {  // Case : 5
    if (!check_address(file)) {
        exit(-1);
    }
    // filesys.c 에 정의된 함수 사용
    return filesys_create(file, initial_size);
}

bool remove(const char *file) {  // Case : 6 -> 일단 간단한 버전
    if (!check_address(file)) {
        exit(-1);
    }
    // filesys.c 에 정의된 함수 사용
    return filesys_remove(file);
}

int open(const char *file_name) {  // Case : 7

    if (!check_address(file_name)) {
        exit(-1);  //유효하지 않은 파일이면 exit
    }

    // 파일 이름을 커널 메모리로 안전하게 복사
    char *file_name_copy = palloc_get_page(0);
    if (file_name_copy == NULL) {
        return -1;  // 커널 메모리 부족
    }

    //복사
    strlcpy(file_name_copy, file_name, PGSIZE);

    // 파일 열기
    struct file *file_ptr = filesys_open(file_name_copy);
    palloc_free_page(file_name_copy);

    if (file_ptr == NULL) {
        return -1;  // 정상적인 실패상황
    }

    // 비어있는 FD 테이블 번호 찾기
    struct thread *cur = thread_current();  //현재 쓰레드 ptr
    int fd = -1;

    // fd 테이블이 없으면 새로 할당
    if (cur->fdt == NULL) {
        cur->fdt = palloc_get_page(PAL_ZERO);  // PAL_ZERO로 초기화

        //예외처리
        if (cur->fdt == NULL) {
            file_close(file_ptr);
            return -1;
        }
    }

    // FD 테이블의 최대번호는 몇으로 해야할까?
    for (int i = 2; i < FDT_MAX_SIZE; i++) {
        if ((cur->fdt)[i] == NULL)  // fd 인덱스가 비어있다면
        {
            cur->fdt[i] = file_ptr;  // fd와 파일 연결
            fd = i;                  // 찾은 fd저장
            break;
        }
    }

    if (fd == -1) {
        file_close(file_ptr);
    }

    return fd;  // 목표한 fd 반환
}

int filesize(int fd) {  // Case : 8 -> read()에서 호출
    int size = 0;

    struct thread *cur_thread = thread_current();

    struct file *cur_file = cur_thread->fdt[fd];

    // 파일이 없을때
    if (cur_file == NULL) {
        return -1;
    }

    // file.c
    size = file_length(cur_file);

    return size;
}

int read(int fd, void *buffer, unsigned size) {  // Case : 9
    // 목표 : fd로 파일을 읽어와서, 버퍼에 size 바이트 만큼 읽어오기

    // KERN_BASE : 0x8004000000 부터 시작
    // read-bad-ptr.c 테스트에서 0xc0100000 주소접근 => 커널영역 접근 제한 필요
    // 헬퍼함수 안에서 is_kernel_vaddr 로 검증
    if (!check_address(buffer) || !check_address(buffer + size - 1)) {
        exit(-1);
    }

    // read-bad-fd.c : fd 범위 벗어나는지 체크
    if (fd < 0 || fd >= FDT_MAX_SIZE) {
        exit(-1);
    }

    struct thread *cur_thread = thread_current();  // 현재 쓰레드
    int bytes_read = 0;                            // 반환값에 쓸, 읽어온 바이트 수

    // fd = 0 : 표준입력 처리 -> 한글자씩 읽어오기
    if (fd == 0) {
        for (int i = 0; i < size; i++) {
            // input_getc : 키보드로부터 문자 하나를 입력받아 반환
            ((char *)buffer)[i] = input_getc();
        }
        bytes_read = size;
    }

    // fd == 1 : 표준 출력 -> read 에선 X
    else if (fd == 1) {
        return -1;
    }

    else {  // fd >= 2 : 파일 읽어오기

        // 1. fd 유효성 검사
        if (fd < 0 || fd >= FDT_MAX_SIZE) {
            return -1;
        }

        // 현재 파일 ptr
        struct file *cur_file = cur_thread->fdt[fd];

        // 파일이 없을 때 예외처리
        if (cur_file == NULL) {
            return -1;
        }

        // 동시접근 막기위한 락 획득
        // lock_acquire(&);

        // file.c 의 file_read 사용
        bytes_read = file_read(cur_file, buffer, size);

        // 락 해제
        // lock_release(&);
    }

    return bytes_read;
}

int write(int fd, const void *buffer, unsigned size) {  // Case : 10
    // 버퍼 주소 유효성 검사
    if (!check_address(buffer)) {
        exit(-1);
    }

    // write-bad-fd.c : fd 범위 벗어나는지 체크
    if (fd < 0 || fd >= FDT_MAX_SIZE) {
        exit(-1);
    }

    int bytes_written = 0;

    // 2. fd == 1: 표준출력 -> 콘솔에 출력
    if (fd == STDOUT_FILENO) {
        // 여러 프로세스의 출력이 섞이는 것을 방지하기 위해
        // 버퍼 전체를 한번에 출력하는 putbuf()를 사용 - GitBook 참고
        putbuf(buffer, size);
        bytes_written = size;
    }

    // 3. fd == 0 : 표준 입력 -> 쓰기불가
    else if (fd == STDIN_FILENO) {
        return -1;
    }

    // 4. 그 외의 fd : fd에 해당하는 파일에 쓰기
    else {
        struct thread *cur = thread_current();  //현재 쓰레드

        // fd가 유효한 범위에 있는지 확인
        if (fd < 2 || fd >= 128) {
            return -1;
        }

        // FD 를 실제 파일 객체로 변환하는 과정 => file_write()에 사용
        struct file *file_obj = cur->fdt[fd];
        if (file_obj == NULL) {
            // 해당하는 fd에 열린 파일이 없는 경우
            return -1;
        }

        // file_write는 파일 끝까지만 쓰고 실제 쓰여진 바이트 수를 반환
        bytes_written = file_write(file_obj, buffer, size);
    }
    return bytes_written;
}

void close(int fd) {  // Case : 13

    // 1. fd 유효성 검사
    if (fd <= 1 || fd >= FDT_MAX_SIZE) {
        exit(-1);
    }

    // 2. 파일찾기
    struct thread *cur = thread_current();
    if (cur->fdt[fd] == NULL) {
        exit(-1);  // fd 테이블에 없으면 에러
    }

    // 3. 커널의 파일 닫기 함수 호출
    file_close(cur->fdt[fd]);  // 여기서 cur->fdr[fd] 가 가리키는 "파일"을 free

    // 4. FDT 슬롯 비우기
    cur->fdt[fd] = NULL;
}

//////////////////////////////////////////////////////////////////
// 07.23 추가 : 유효주소 검사하는 헬퍼 함수 => exec() 에서 사용
static bool check_address(void *addr) {
    // NULL 포인터 체크
    if (addr == NULL) {
        return false;
    }

    // 커널 주소 영역 체크
    if (is_kernel_vaddr(addr)) {
        return false;
    }

    // 현재 스레드의 페이지 테이블에서 해당 주소가 매핑되어 있는지 확인
    struct thread *cur = thread_current();
    if (cur->pml4 == NULL) {
        return false;
    }

    void *page = pml4_get_page(cur->pml4, addr);
    if (page == NULL) {
        return false;
    }

    return true;
}