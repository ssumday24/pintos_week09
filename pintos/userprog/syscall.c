#include "userprog/syscall.h"

#include <stdio.h>
#include <syscall-nr.h>

#include "intrinsic.h"
#include "threads/flags.h"
#include "threads/init.h" /* pintos/include/threads/init.h*/
#include "threads/interrupt.h"
#include "threads/loader.h"
#include "threads/thread.h"
#include "userprog/gdt.h"

/* 헤더 파일 , 전연 락 변수 추가 07.22 */
#include "filesys/file.h"
#include "filesys/filesys.h"
#include "threads/synch.h"

/* 필요한 함수 선언 추가 07.23*/
static bool check_address(void *addr);

void syscall_entry(void);
void syscall_handler(struct intr_frame *);

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

/* ======= 필요한 함수 선언 부분 ==============*/
void halt(void);
void exit(int status);
void exec(const char *cmd_line);
int wait(tid_t pid);
int write(int fd, const void *buffer, unsigned size);

#define STDIN_FILENO 0
#define STDOUT_FILENO 1
/* ======= 필요한 함수 선언 부분 ==============*/

/* 메인 시스템콜 인터페이스  => 커널공간  */
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
        // case SYS_WAIT:  // case : 4
        //     f->R.rax = wait (f->R.rdi);
        //     break;
        // case SYS_CREATE:  // case : 5
        //     f->R.rax = create (f->R.rdi, f->R.rsi);
        //     break;
        // case SYS_REMOVE:  // case : 6
        //     f->R.rax = remove (f->R.rdi);
        //     break;
        // case SYS_OPEN:  // case : 7
        //     f->R.rax = open (f->R.rdi);
        //     break;
        // case SYS_FILESIZE:  // case : 8
        //     f->R.rax = filesize (f->R.rdi);
        //     break;
        // case SYS_READ:  // case : 9
        //     f->R.rax = read (f->R.rdi, f->R.rsi, f->R.rdx);
        //     break;
        case SYS_WRITE:  // case : 10
            f->R.rax = write(f->R.rdi, f->R.rsi, f->R.rdx);
            break;
        // case SYS_SEEK:
        //     seek (f->R.rdi, f->R.rsi);
        //     break;
        // case SYS_TELL:
        //     f->R.rax = tell (f->R.rdi);
        //     break;
        // case SYS_CLOSE:
        //     close (f->R.rdi);
        //     break;
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
//     return -1;
// }

void exec(const char *cmd_line) {  // Case : 3
    // cmd_line에 주어진 이름을 가진 실행파일로 현재 프로세스 변경하고 주어진 인자 전달
    //  반환값 :
    //  성공: 반환 안 됨
    //  실패: 프로세스 종료 및 종료 상태 -1 반환

    if (!check_address((void *)cmd_line)) {
        exit(-1);
    }
    // ====== 주소 검사 부분 =======
    const char *p = cmd_line;

    while (true) {
        if (!check_address((void *)p)) {
            exit(-1);
        }
        if (*p == '\0') {
            break;
        }
        p++;
    }

    char *cmd_line_copy;  // process_exec()에서 파싱해야하는데, cmd_line은 const => 복사본 만들기
    cmd_line_copy = palloc_get_page(0);  // 단일 페이지 할당

    printf("%p\n", cmd_line_copy);
    if (cmd_line_copy == NULL) {
        exit(-1);
    }
    printf("MY NEW KERNEL IS DEFINITELY RUNNING!!!!");
    strlcpy(cmd_line_copy, cmd_line, PGSIZE);  // cmd_line copy

    // ===== 현재 실행 프로세스 내용 파괴 & 덮어쓰기 =====
    if (process_exec(cmd_line_copy) == -1) {
        exit(-1);  // 실패시 -1 로 종료
    }

    //도달불가
}

// int wait(tid_t pid) {  // Case : 4
//     return -1;
// }

// bool create(const char *file, unsigned initial_size) {  // Case : 5
//     if (check_address(file)) {
//         return -1;
//     }
//     return filesys_create(file,initial_size);
// }

// bool remove(const char *file) {  // Case : 6
//     return -1;
// }

// int open(const char *file) {  // Case : 7
//     return -1;
// }

// int filesize(int fd) {  // Case : 8
//     return -1;
// }

// int read(int fd, void *buffer, unsigned size) {  // Case : 9
//     return -1;
// }

int write(int fd, const void *buffer, unsigned size) {  // Case : 10
    // 1. 버퍼 주소의 유효성 검사
    if (!check_address(buffer)) {
        exit(-1);  // 유효하지 않으면 exit
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