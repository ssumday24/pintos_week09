# 소스 코드 정보

- threads/: 기본 커널 소스 코드입니다. 프로젝트 1부터 해당 코드를 수정하게 됩니다.
- userprog/: 사용자 프로그램 로더 소스 코드입니다. 프로젝트 2부터 이 디렉터리의 코드를 수정합니다.
- vm/: 거의 비어 있는 디렉터리입니다. 프로젝트 3에서 가상 메모리 구현을 이곳에 작성합니다.

- filesys/: 기본 파일 시스템 소스 코드입니다. 프로젝트 2부터 사용하지만, 프로젝트 4 이전에는 수정하지 않습니다.
- devices/: 키보드, 타이머, 디스크 등 I/O 장치 인터페이스 소스 코드입니다. 프로젝트 1에서는 타이머 구현만 수정하며, 그 외에는 변경할 필요가 없습니다.
- lib/: 표준 C 라이브러리의 부분 구현체입니다. Pintos 커널과(프로젝트 2부터 실행되는 사용자 프로그램에도) 컴파일되며, 수정할 일은 거의 없습니다.
- include/lib/kernel/: 커널 전용 C 라이브러리 헤더와 비트맵, 이중 연결 리스트, 해시 테이블 등의 데이터 구조 구현이 포함되어 있습니다. 커널에서는 #include <…>로 포함할 수 있습니다.
- include/lib/user/: 사용자 프로그램 전용 C 라이브러리 헤더입니다. 사용자 프로그램에서는 #include <…>로 포함할 수 있습니다.
- tests/: 각 프로젝트별 테스트 코드입니다. 테스트 용도로 수정할 수 있으나, 최종 제출 전에는 원본으로 복원됩니다.
- examples/: 프로젝트 2부터 사용할 예제 사용자 프로그램이 포함되어 있습니다.
- include/: 헤더 파일(*.h) 소스 코드가 포함된 디렉터리입니다.

# 실행방법

Pintos 실행 방법은 다음과 같습니다.

1. **pintos 프로그램**
   Pintos를 시뮬레이터에서 편리하게 실행하기 위해 `pintos`라는 실행 스크립트를 제공합니다. 가장 간단한 형태로는

   ```
   pintos 인수...
   ```

   와 같이 호출할 수 있으며, 각 인수가 Pintos 커널에 그대로 전달되어 해당 기능을 수행합니다.

2. **빌드 디렉터리로 이동 후 테스트 실행**
   새로 생성된 `build/` 디렉터리로 이동한 뒤, 다음과 같이 입력하십시오.

   ```
   pintos run alarm-multiple
   ```

   여기서 `run`은 커널에 테스트를 실행하라는 명령이며, `alarm-multiple`은 실행할 테스트 이름입니다. 이 명령을 실행하면 Pintos가 부팅되어 `alarm-multiple` 테스트 프로그램을
   수행하고, 여러 화면 분량의 텍스트를 출력합니다.

3. **직렬 출력을 파일로 저장**
   직렬 출력(serial output)을 파일로 기록하려면 셸 리디렉션을 이용합니다.

   ```
   pintos -- run alarm-multiple > logfile
   ```

   위와 같이 `--` 이후에 커널 인수를 적고, 그 앞에 옵션을 배치해야 합니다.

4. **추가 옵션 지정 방법**
   `pintos` 프로그램은 QEMU 또는 가상 하드웨어를 제어하는 여러 옵션을 지원합니다. 옵션을 지정할 때는 반드시 커널 명령보다 앞에 두고, `--` 로 구분해야 합니다. 전체 명령어 형식은 다음과
   같습니다.

   ```
   pintos [옵션…] -- [커널 인수…]
   ```

   인수 없이 `pintos`만 실행하면 사용 가능한 옵션 목록이 표시됩니다. 주요 옵션 예시는 다음과 같습니다.

    * `-v` : VGA 디스플레이를 끕니다.
    * `-s` : stdin에서의 직렬 입력과 stdout으로의 직렬 출력을 생략합니다.

5. **커널 내장 명령 및 도움말**
   Pintos 커널은 `run` 이외에도 여러 명령과 옵션을 제공합니다. 당장 자주 사용할 기능은 아니지만, 전체 목록을 확인하려면

   ```
   pintos -h
   ```

   를 실행하십시오.

# week1

## 관련 소스 코드

- 주로 threads/ 디렉터리에서 작업
- 부차적으로 devices/ 디렉터리의 일부를 수정

## 참고

- [Synchronization.url](..%2F..%2F..%2F..%2FAppData%2FLocal%2FTemp%2FSynchronization.url);

## 배경지식

초기 스레드 시스템 코드를 읽고 이해 -> 스레드 생성 및 종료, 스레드 간 전환을 수행하는 간단한 스케줄러, 그리고 동기화 원시(세마포어, 락, 조건 변수, 최적화 배리어)

- 스레드 생성
- 새로운 context가 스케줄러에 등록
- `tid_t thread_create(const char *name, int priority, thread_func *function, void *aux);`
- thread_func: 스레드가 최초로 스케줄러에 의해 선택될 때, 실행될 함수. 해당 함수가 종료되면 스레드가 종료됨 ==> 스레드의 main() 함수 느낌
- 스레드 실행: 항상 하나의 스레드만 실행 / 나머지는 비활성화
- 스케줄러
- 다음 실행할 스레드 결정
- "실행할 준비(ready)된 스레드”가 하나도 없으면, 특별한 유휴 스레드를 띄워 놓음
- idle thread(유휴 스레드): idle() 함수 안에서 무한 루프를 돌면서, 언제라도 실행 가능한 스레드가 나타나면(예: 다른 스레드가 락을 풀어주거나, 타이머 인터럽트가 발생해 깨어나면) 빠르게 제어권을
  넘김
- Synchronization primitives(동기화 프리미티브): 다중 스레드 환경에서 여러 스레드가 공유 자원에 안전하게 접근하고 실행 순서를 조정하기 위해 커널이 제공하는 기본 도구 (세마포어, 락,
  조건변수 등)
- 동기화 원시가 스레드를 block/unblock 시킴
- 어떤 스레드가 자원을 얻거나(또는 어떤 상태를 기다리느라) 스스로 ‘멈춤(block)’ 상태가 되어야 할 때가 있는데 이때 Pintos는 다음 과정을 거칩니다.

1. 해당 스레드가 thread_block() 호출로 자신을 블록 상태로 전환
2. 스케줄러가 즉시 다른 ready 상태의 스레드를 골라 thread_launch()를 통해 실행 컨텍스트를 바꿈
3. 나중에 자원이 해제되거나 조건이 충족되면 thread_unblock()을 호출해 스레드를 다시 ready 상태로 만들고, 스케줄러가 다시 선택할 수 있게 함

- 컨텍스트 전환
    - 실제 동작: threads/thread.c 파일의 `thread_launch()` 함수 참고
    - 현재 실행 중인 스레드의 상태를 저장
    - 전환 대상 스레드의 상태를 복원
    - 한 스레드가 do_iret()의 iret를 실행하는 순간 다른 스레드가 동작을 시작
- Pintos에서는 각 스레드에 약 4 KB 미만의 고정 크기 실행 스택을 할당
    - `int buf[1000];` 같은 큰 배열을 지역 변수로 선언하면 원인 불명의 커널 패닉이 발생할 수 있음

- Synchronization primitives(동기화 프리미티브)
    - 세마포어
        - 정수형 카운터와 그 값을 감소시키는 연산과 증가시키는 연산을 제공
        -

## 소스 코드

### threads 디렉터리:

- loader.S, loader.h:
  커널 로더(bootloader) 코드입니다. BIOS가 메모리에 로드한 512바이트 크기의 초기 코드를 구성하며, 디스크에서 커널을 찾아 메모리에 올린 후 start.S의 bootstrap()으로 분기합니다.
  수정할 필요가 없습니다.
- kernel.lds.S: 커널 링커 스크립트입니다. 커널 이미지의 로드 주소를 설정하고 start.S가 이미지 시작 부분 근처에 위치하도록 배치합니다. 호기심이 없다면 볼 필요는 없습니다.
- init.c, init.h: 커널 초기화 코드로, main() 함수가 포함되어 있습니다. 어떤 컴포넌트가 초기화되는지 확인하려면 main() 부분을 살펴보십시오. 필요하다면 별도 초기화 코드를 추가할 수
  있습니다.
- thread.c, thread.h: 기본 스레드 지원 코드입니다. 이 프로젝트 전반에 걸쳐 가장 많이 수정하게 될 파일로, struct thread 정의가 thread.h에 있습니다.
- palloc.c, palloc.h: 페이지 할당기(page allocator)로, 4 KB 단위 페이지를 시스템 메모리에서 할당해 줍니다.
- malloc.c, malloc.h: 커널용 간단한 malloc()/free() 구현체(block allocator)입니다.
- interrupt.c, interrupt.h: 기본 인터럽트 처리와 인터럽트 활성화/비활성화 함수가 정의되어 있습니다.
- intr-stubs.S, intr-stubs.h: 저수준 인터럽트 처리용 어셈블리 코드입니다.
- synch.c, synch.h: 세마포어, 락, 조건 변수 등 동기화 프리미티브가 구현되어 있습니다. 모든 프로젝트에서 사용하게 됩니다.
- mmu.c, mmu.h: x86-64 페이지 테이블 조작 함수입니다. 3단계(가상 메모리) 과제에서 자세히 다룰 예정입니다.
- io.h: I/O 포트 접근 함수입니다. 대부분 devices/ 코드에서 사용하며, 직접 손댈 필요는 없습니다.
- vaddr.h, pte.h: 가상 주소 및 페이지 테이블 엔트리 관련 매크로와 함수입니다. 3단계 과제 전까지는 무시해도 됩니다.
- flags.h: x86-64 플래그 레지스터 비트 매크로를 정의합니다. 관심이 없다면 넘어가도 됩니다.

### devices 디렉터리: timer.c, timer.h

- **timer.c, timer.h: 기본 시스템 타이머(초당 100틱 동작)입니다. 이 과제에서 수정합니다.**
- vga.c, vga.h: VGA 디스플레이 드라이버로, 화면에 텍스트를 출력합니다. printf()가 이 코드를 호출하므로 직접 건드릴 일은 없습니다.
- serial.c, serial.h: 직렬 포트 드라이버입니다. printf()가 시리얼 입출력을 처리하므로 수정할 필요가 없습니다.
- block.c, block.h: 블록 디바이스(IDE 디스크, 파티션)를 추상화한 코드입니다. 2단계(파일 시스템) 과제에서 사용됩니다.
- ide.c, ide.h: IDE 디스크 섹터 입출력 지원 코드입니다.
- partition.c, partition.h: 디스크 파티션 구조를 이해하고 관리하는 코드입니다.
- kbd.c, kbd.h: 키보드 드라이버로, 키 입력을 입력 계층(input layer)에 전달합니다.
- input.c, input.h: 키보드·시리얼 드라이버로부터 전달된 문자를 큐잉하는 입력 계층입니다.
- intq.c, intq.h: 커널 스레드와 인터럽트 핸들러가 동시에 접근하는 원형 큐(인터럽트 큐) 구현입니다.
- rtc.c, rtc.h: 실시간 시계 드라이버로, 커널 초기화 시 난수 시드 결정 등에 사용됩니다.
- speaker.c, speaker.h: PC 스피커로 음을 출력하는 드라이버입니다.
- pit.c, pit.h: 8254 프로그래머블 인터럽트 타이머(PIT) 구성 코드로, timer.c와 speaker.c에서 각각 한 채널씩 사용합니다.

### lib 디렉터리

- ctype.h, inttypes.h, limits.h, stdarg.h, stdbool.h, stddef.h, stdint.h, stdio.c, stdio.h, stdlib.c, stdlib.h,
  string.c, string.h: 표준 C 라이브러리의 부분 구현체입니다.
- debug.c, debug.h: 디버깅 보조 함수 및 매크로입니다.
- random.c, random.h: 의사 난수 생성기입니다. 실행마다 동일한 시퀀스를 생성합니다.
- round.h: 반올림 매크로 정의입니다.
- syscall-nr.h: 시스템 호출 번호 정의입니다. 2단계 과제에서 사용합니다.
- kernel/list.c, kernel/list.h: 이중 연결 리스트 구현체로, 1단계 과제에서 직접 사용하게 될 가능성이 높습니다.
- kernel/bitmap.c, kernel/bitmap.h: 비트맵 구현체입니다. 필요하면 사용 가능합니다.
- kernel/hash.c, kernel/hash.h: 해시 테이블 구현체로, 3단계 과제에서 유용합니다.
- kernel/console.c, kernel/console.h, kernel/stdio.h: printf() 등 콘솔 출력 기능을 구현합니다.

## 동기화

- 인터럽트와 동기화문제
    - 인터럽트(interrupt): 하드웨어나 타이머, I/O 장치 등에서 “지금 바로 처리해야 할 일”이 생겼을 때 CPU에 알리는 신호
    - 인터럽트가 발생하면 평소 실행하던 코드가 언제든지 “뚝” 끊기고 잠시 다른 코드가 실행되므로, 인터럽트가 활성화된 상태에서는 사실상 두 흐름이 섞여 동작하게 됨
    - 해결 방법
        - 인터럽트 비활성화
            - only 커널 스레드와 인터럽트 핸들러간에 공유되는 데이터의 동기화
            - 인터럽트 핸들러에는 lock을 걸 수 없음 -> sleep 할 수 없기 때문에.
            - 둘이 함께 쓰는 데이터는 커널 스레드 쪽에서 인터럽트를 끄고 보호해야함.
            - 실제 경우
                - 알람 시계(alarm clock) 구현에서는 타이머 인터럽트가 잠들어 있는 스레드를 깨워야 합니다.
                - 고급 스케줄러(advanced scheduler) 구현에서는 타이머 인터럽트가 몇몇 전역 변수 및 스레드별 변수를 참조해야 합니다.
                - 커널 스레드에서 이들 변수를 접근할 때는, 타이머 인터럽트가 방해하지 않도록 반드시 인터럽트를 비활성화해하나 또한 **최소화** 해야함
        - 동기화 원시: 세마포어, 락, 조건 변수
            - 얘도 사실은 내부적으로 인터럽트 비활성화를 사용
        - 바쁜 대기는 절대 사용해서는 안 됨. (ex. thread_yield())

## 할일

### Alarm Clock

```c
/* Suspends execution for approximately TICKS timer ticks. */
void timer_sleep(int64_t ticks) { // 스레드를 일정시간동안 대기상태로 두고 싶을 때 주로 사용하는 함수임.
    int64_t start = timer_ticks();

    ASSERT(intr_get_level() == INTR_ON);
    while (timer_elapsed(start) < ticks) thread_yield();
}
```

- `devices/timer.c`에 정의된 `timer_sleep()` 함수
- 현재는 바쁜 대기 방식 사용 -> 현재 시간을 반복해서 확인하고 `thread_yield()` 호출 하는데 이를 회피하도록 재구현해야함.
- 내 생각에는
    - 스레드를 잠깐 block 상태로 두고,

#### 관련 코드 분석

```c
// thread.h
struct thread {
	/* Owned by thread.c. */
	tid_t tid;                          /* Thread identifier. */
	enum thread_status status;          /* Thread state. */
	char name[16];                      /* Name (for debugging purposes). */
	int priority;                       /* Priority. */

	/* Shared between thread.c and synch.c. */
	struct list_elem elem;              /* List element. */

#ifdef USERPROG
	/* Owned by userprog/process.c. */
	uint64_t *pml4;                     /* Page map level 4 */
#endif
#ifdef VM
	/* Table for whole virtual memory owned by thread. */
	struct supplemental_page_table spt;
#endif

	/* Owned by thread.c. */
	struct intr_frame tf;               /* Information for switching */
	unsigned magic;                     /* Detects stack overflow. */
};
```

```c
// thread.c

static long long idle_ticks;   /* # of timer ticks spent idle. */
static long long kernel_ticks; /* # of timer ticks in kernel threads. */
static long long user_ticks;   /* # of timer ticks in user programs. */

/* Scheduling. */
#define TIME_SLICE 4          /* # of timer ticks to give each thread. */
static unsigned thread_ticks; /* # of timer ticks since last yield. */

/* Yields the CPU.  The current thread is not put to sleep and
   may be scheduled again immediately at the scheduler's whim. */
void thread_yield(void) {
    struct thread *curr = thread_current(); // 현재 스레드
    enum intr_level old_level;  // interrupt 활성화 비활성화 여부

    ASSERT(!intr_context());

    old_level = intr_disable(); // interrupt를 비활성화 시키고 이전 interrupt 상태를 반환
    if (curr != idle_thread)        // 현재 스레드가 유휴 스레드가 아니면,
        list_push_back(&ready_list, &curr->elem);   // 현재 스레드를 list의 맨 마지막에 넣음
    do_schedule(THREAD_READY);  //
    intr_set_level(old_level);
}


/* Schedules a new process. At entry, interrupts must be off.
 * This function modify current thread's status to status and then
 * finds another thread to run and switches to it.
 * It's not safe to call printf() in the schedule(). */
static void do_schedule(int status) {
    ASSERT(intr_get_level() == INTR_OFF);
    ASSERT(thread_current()->status == THREAD_RUNNING);
    while (!list_empty(&destruction_req)) { // 소멸(회수)되어야할 스레드의 리스트가 있으면,
        struct thread *victim = list_entry(list_pop_front(&destruction_req), struct thread, elem);  // 그 중에 한 스레드 가져옴
        palloc_free_page(victim);   // 아마 그 스레드 취소하는 거?
    }
    thread_current()->status = status;  // 현재 스레드 상태를 주어진 상태로 변경
    schedule();
}

static void schedule(void) {
    struct thread *curr = running_thread();
    struct thread *next = next_thread_to_run();

    ASSERT(intr_get_level() == INTR_OFF);
    ASSERT(curr->status != THREAD_RUNNING);
    ASSERT(is_thread(next));
    /* Mark us as running. */
    next->status = THREAD_RUNNING;  // 다음 걸 실행되게 만들고

    /* Start new time slice. */
    thread_ticks = 0;

#ifdef USERPROG
    /* Activate the new address space. */
    process_activate(next);
#endif

    if (curr != next) {
        /* If the thread we switched from is dying, destroy its struct
           thread. This must happen late so that thread_exit() doesn't
           pull out the rug under itself.
           We just queuing the page free reqeust here because the page is
           currently used by the stack.
           The real destruction logic will be called at the beginning of the
           schedule(). */
        if (curr && curr->status == THREAD_DYING && curr != initial_thread) {
            ASSERT(curr != next);
            list_push_back(&destruction_req, &curr->elem);
        }

        /* Before switching the thread, we first save the information
         * of current running. */
        thread_launch(next);
    }
}

/* Called by the timer interrupt handler at each timer tick.
   Thus, this function runs in an external interrupt context. */
void thread_tick(void) {
    struct thread *t = thread_current();

    /* Update statistics. */
    if (t == idle_thread)
        idle_ticks++;
#ifdef USERPROG
    else if (t->pml4 != NULL)
        user_ticks++;
#endif
    else
        kernel_ticks++;

    /* Enforce preemption. */
    if (++thread_ticks >= TIME_SLICE)   // 선점형인가봄. 기본적으로 thread_ticks이 정해진 시간(TIME_SLICE)를 넘으면
        intr_yield_on_return();     // 이부분 잘 모르겠음.
}
```

```c
// list.h
/* Converts pointer to list element LIST_ELEM into a pointer to
   the structure that LIST_ELEM is embedded inside.  Supply the
   name of the outer structure STRUCT and the member name MEMBER
   of the list element.  See the big comment at the top of the
   file for an example. */
#define list_entry(LIST_ELEM, STRUCT, MEMBER)           \
	((STRUCT *) ((uint8_t *) &(LIST_ELEM)->next     \
		- offsetof (STRUCT, MEMBER.next)))
```

#### 실행 흐름

1. tick interrupt 발생
2. `timer_interrupt()` 핸들러 호출
    - `ticks++`
    - `thread_tick()` 호출
3. `thread_tick()` 호출
    - 각 상황에 맞는 ticks ++
    - thread_ticks가 지정된 시간(TIME_SLICE) 보다 늘었으면
    - 지금 처리 중인 인터럽트가 완전히 끝나고 나서 곧바로 thread_yield()를 호출하는 flag 세팅

---

1. `timer_sleep()` 호출
2. 자야하는 시간이면, `thread_yield()` 호출
    - 근데 `thread_yield()`는
        - 여전히 현재 스레드(sleep 상태)를 ready_list에 넣음
        - 현재 스레드를 ready 상태로 바꾸고
        - 새로운 스레드를 running 상태로 바꿈
3. 그 안에서 `schedule()` 호출
    - ready_list에서 다음으로 실행할 애 하나 찾아와서 상태를 running으로 변경 (없으면 idle_thread)
    - `thread_launch()`를 호출해 next 스래드 실행

---

- 문제
    - 자야하는 스레드가 ready list에 있으니 계속 호출될 수 있다는 문제
        - block list와 block 상태 만들기
    - 자는 애를 어떻게 깨운는 지 문제
        - tick이 왔을 때 block list 돌면서 block 풀어주기.

### Priority Scheduling

- 우선순위에 따른 즉시 선점
    - 새 스레드가 준비 큐에 추가될 때, 실행 중인 스레드보다 우선순위가 높다면 즉시 cpu를 양보해야암
    - 깨울 때도, 우선순위가 높은 스레드를 먼저 깨워야함.
    - 우선 순위는 변경 가능 -> 이에 따라 즉시 양보가 필요할 수도.
- 범위
    - 0 ~ 63
    - 스레드 생성 시 thread_create()의 인자로 초기 우선순위를 지정.
    - 특별한 이유 없으면 31 사용.
- 우선순위 기부
    - 상위 우선순위 스레드(H)가 하위 우선순위 스레드(L)에 엮여 있을 때
    - H는 L에 자신의 우선순위를 기부해 줄 수 있음.
    - 기부된 우선순위는 L이 락을 해제하고 H가 락을 획득하면 회수됨.
    - 중첩 기부도 처리해야함. -> 기부 받은 모든 스레드는 기부한 스레드의 우선순위로 상승
- 구현
    - 범위
        - 우선순위 스케줄링: 모든 경우
        - 우선순위 기부: lock에 대해서만
            - 락: 스레드들이 동시에 같은 자원을 건드리지 않게 하기 위해
                - 누구든 자원을 사용하려면 락을 획득(lock_acquire)하고, 다 쓰면 락을 반납(lock_release)해야 함.
    - 관련 함수
        - `thread_set_priority()`: 현재 스레드의 우선순위를 new_priority로 설정 / 양보가 필요하면 양보
        - `thread_get_priority()`: 현재스레드의 우선순위를 반환 / 우선순위 기부가 있다면 기부된 우선순위 반환

#### gitbook

- 동기화: 스레드 간 자원을 공유할 때 필요한 처리.
    - 인터럽트 비활성화
        - 가장 원시적인 방법
        - 인터럽트가 꺼져있으면 다른 스레드에 의해 선점되지 않음
        - 주로, 커널 스레드와 인터럽트 핸들러 함수 간의 동기화를 위해 사용된다.
            - 인터럽트 핸들러는 빠른 시간에 완료되어야하기 때문에, 락을 걸기에 부적합함.
- 세마포어
    - 음이 아닌 정수와 그 숫자를 다루는 두가지 연산으로 이루오진 동기화 도구
    - 원자적임
    - 연산
        - down
            - 세마포어 값이 양수가 될 때까지 기다린다
            - 값이 양수가 되면, 값을 1 감소시킨다.
        - UP
            - 값을 1증가시킨다
            - 기다리고 있는 스레드가 있으면, 그 중 하나를 깨워서 실행하게 한다.
    - 세마포어를 0으로 초기화: 특정 이벤트가 한번 일어나길 기다리는 상황
        - A 스레드가 B 스레드를 생성한다.
        - A는 B가 작업을 끝날 때까지 기다리고 싶다.
        - 그러면 A는 down을 호출하고
        - B로 넘겨서 B는 할일을 하고 sema를 1올려서 A한테 넘겨줌
    - 세마포어를 1로 초기화한 경우: 자원을 하나만 허용하는 경우
        - 자원 사용 전에, down을. 자원 사용 후에 up을 함
        - 뮤텍스와 비슷하며, 보통 lock이 더 적합함.
    - 세마포어를 2 이상으로 초기화하는 경우
        - 여러 개의 동일한 자원을 동시에 허용하고 싶을 때 사용
        - 실제로는 거의 사용하지 않음.
    - 주의
        - sema_try_down: 기다리지 않고 down 연산을 시도(성공: true, 실패: false), CPU 낭비. 내가 보기엔 busy wait 방식임
        - sema_up: 특이하게 인터럽트 핸들러에서 사용을 할 수 있는데 그 이유는...
            - '기다리는 연산'을 하지 않음
            - <세마포어가 0으로 세팅 = 특정 이벤트를 기다림> 인데, 키보드 인터럽트가 들어온 경우 읽기 작업을 기다리던 스레드를 깨워야해!
              라는 의미로 sema_up을 할 수 있음
    - 내부 동작
        - 인터럽트 비활성화 필요. (원자적이어야하니까)
        - 스레드 block/unblock
        - 세마포어를 기다리는 스레드: 리스트에 저장
- 락
    - 초기값이 1인 세마포어와 유사
    - 변수
        - holder: lock을 얻은 소유자 스레드 정보
        - sema
    - 연산
        - release <= up
        - acquire <= down
    - 추가 제약
        - 락을 획득한 스레드만 이 해당 락을 해제할 ㅜㅅ 있음
    - 재귀적인 락이 아니므로 락을 가진 스레드가 같은 락을 획득하려해서는 안됨.

- 모니터
    - 세마포어나 락보다 더 높은수준의 동기화 방식
    - 구성
        - 동기화할 데이터
        - 락(모니터락)
        - 하나이상의 조건 변수
    - 동작
        - 임계구역에 들어가기 위해 모니터 락 획득 -> 락 획득 == 모니터 안에 들어갔다
        - 모니터 안에서는 보호된 데이터를 마음껏 사용.
        - 데이터 사용을 마치면 락을 해제, 모니터에서 나옴
    - 조건 변수: 모니터 안에서 조건을 기다려야할 때,
        - 특정 조건이 만족될 때까지 기다리게 하는 역할.
        - 조건 변수에 대해 wait을 호출 => 조건 변수 == 세마포어?
        - 이때 lock을 자동으로 해제하고, 조건이 signal 될 때까지 기다림.
        - 조건이 만족되면 다시 lock을 획들하고 실행을 계속
    - 내가 이해한 바로는 초기값이 0인 세마포어 느낌도 있음....

#### 코드 분석

```c
// synch.h
/* A counting semaphore. */
struct semaphore {
	unsigned value;             /* Current value. */
	struct list waiters;        /* List of waiting threads. */
};

/* Lock. */
struct lock {
	struct thread *holder;      /* Thread holding lock (for debugging). */
	struct semaphore semaphore; /* Binary semaphore controlling access. */
};

/* Condition variable. */
struct condition {
	struct list waiters;        /* List of waiting threads. */
};
```