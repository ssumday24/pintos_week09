---
name: Feature Request # 이슈 템플릿의 이름입니다. 깃허브 이슈 생성 시 이 이름이 보입니다.
about: Suggest an idea for a new feature or enhancement for the project. # 이 템플릿이 어떤 용도인지 간략히 설명합니다.
title: "[Feature]: Your concise feature request title here" # 새 이슈 생성 시 기본 제목으로 표시됩니다.
labels: enhancement # 이 이슈에 자동으로 할당될 라벨입니다. (리포지토리에 'enhancement' 라벨이 있어야 합니다.)
assignees: ssumday24 # 이 이슈에 자동으로 할당될 담당자입니다. 
---

### ✨ Is your feature request related to a problem? Please describe.

[새로운 기능을 제안하게 된 배경이 되는 **현재의 문제점**이나 **불편함**이 있다면 설명해주세요. 이 기능이 어떤 문제를 해결하는 데 도움이 될지 명확히 해주세요. 
예: "현재 Pintos의 파일 시스템은 동시 접근 시 데이터 무결성을 보장하지 못하여 여러 프로세스가 동시에 파일을 읽고 쓸 때 데이터가 손상될 위험이 있습니다."]

---

### 🚀 Describe the solution you'd like

[원하는 **새로운 기능**에 대해 명확하고 간결하게 설명해주세요. 어떻게 동작하기를 바라는지, 어떤 변화가 있을 것인지 구체적으로 작성합니다.
예: "파일 시스템 접근 시 락(lock) 메커니즘을 도입하여, 한 번에 하나의 스레드/프로세스만 파일 시스템 함수에 접근하도록 해야 합니다. 이는 `filesys/filesys.c`의 주요 함수들에 적용되어야 합니다."]

---

### 🔄 Describe alternatives you've considered (Optional)

[이 기능을 구현하기 위해 **고려했거나 알고 있는 다른 접근 방식이나 대안**이 있다면 설명해주세요. 각 대안의 장단점도 포함하면 좋습니다.
예: "세마포어를 사용하는 방법도 고려했으나, 파일 시스템 전체에 대한 상호 배제(mutual exclusion)를 위해서는 락이 더 적합하다고 판단했습니다."]

---

### 💡 Additional Context (Optional)

[이 기능과 관련하여 추가적으로 제공하고 싶은 정보가 있다면 작성해주세요. 예를 들어:
* 이 기능이 구현되면 얻을 수 있는 **이점**.
* 예상되는 **구현 난이도**나 **복잡성**.
* 참조할 만한 **외부 자료(링크)**나 **코드 스니펫**.
예: "이 기능은 Project 2의 동기화 요구사항을 충족하며, 향후 Project 3 및 Project 4의 파일 시스템 작업의 안정성을 높이는 데 필수적입니다."]

---