#ifndef THREADS_INTERRUPT_H
#define THREADS_INTERRUPT_H

#include <stdbool.h>
#include <stdint.h>

/* Interrupts on or off? */
enum intr_level {
    INTR_OFF, /* Interrupts disabled. */
    INTR_ON   /* Interrupts enabled. */
};

enum intr_level intr_get_level(void);
enum intr_level intr_set_level(enum intr_level);
enum intr_level intr_enable(void);
enum intr_level intr_disable(void);

/* Interrupt stack frame. */
struct gp_registers {
    uint64_t r15;
    uint64_t r14;
    uint64_t r13;
    uint64_t r12;
    uint64_t r11;
    uint64_t r10;
    uint64_t r9;
    uint64_t r8;
    uint64_t rsi;
    uint64_t rdi;
    uint64_t rbp;
    uint64_t rdx;
    uint64_t rcx;
    uint64_t rbx;
    uint64_t rax;
} __attribute__((packed));

struct intr_frame {
    /* Pushed by intr_entry in intr-stubs.S.
     * These are the interrupted task's saved registers. */
    struct gp_registers
        R;  // 일반 목적 레지스터들 (General Purpose Registers) - rax, rbx, rcx, rdx,등
    uint16_t es;      // Extra Segment (데이터 세그먼트 레지스터)
    uint16_t __pad1;  // 정렬(padding)을 위한 더미 변수
    uint32_t __pad2;  // 정렬(padding)을 위한 더미 변수
    uint16_t ds;      // Data Segment (데이터 세그먼트 레지스터)
    uint16_t __pad3;  // 정렬(padding)을 위한 더미 변수
    uint32_t __pad4;  // 정렬(padding)을 위한 더미 변수
    /* Pushed by intrNN_stub in intr-stubs.S. */
    uint64_t vec_no;  // 인터럽트/예외 벡터 번호 (어떤 인터럽트가 발생했는지 식별)
                      // intr-stubs.S에서 각 인터럽트 스텁이 푸시.
    uint64_t
        error_code;  // CPU에 의해 푸시되는 오류 코드 (예외 발생 시에만 유효)
                     // 오류 코드가 없는 인터럽트의 경우 intrNN_stub에서 0을 푸시하여 일관성을 유지.
    /* Pushed by the CPU.
     * These are the interrupted task's saved registers. */
    uintptr_t rip;    // Instruction Pointer (다음으로 실행할 명령의 주소)
                      // 인터럽트 발생 시 중단된 명령의 다음 명령 주소.
    uint16_t cs;      // Code Segment (코드 세그먼트 레지스터)
    uint16_t __pad5;  // 정렬을 위한 더미 변수
    uint32_t __pad6;  // 정렬을 위한 더미 변수
    uint64_t eflags;  // EFLAGS 레지스터 (CPU 상태 플래그들)
                      // 인터럽트 가능 여부, 캐리 플래그 등 포함.
    uintptr_t rsp;    // Stack Pointer (현재 스택의 최상단 주소)
                      // 인터럽트 발생 시 스택 포인터.
    uint16_t ss;      // Stack Segment (스택 세그먼트 레지스터)
    uint16_t __pad7;  // 정렬을 위한 더미 변수
    uint32_t __pad8;  // 정렬을 위한 더미 변
} __attribute__((packed));  // 구조체 멤버 간의 패딩을 최소화하여 메모리 공간을 절약.

typedef void intr_handler_func(struct intr_frame *);

void intr_init(void);
void intr_register_ext(uint8_t vec, intr_handler_func *, const char *name);
void intr_register_int(uint8_t vec, int dpl, enum intr_level, intr_handler_func *,
                       const char *name);
bool intr_context(void);
void intr_yield_on_return(void);

void intr_dump_frame(const struct intr_frame *);
const char *intr_name(uint8_t vec);

#endif /* threads/interrupt.h */
