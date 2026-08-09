# CBR300R 단기통 엔진 적용 노트

이 문서는 Honda CBR300R(286cc, 단기통, 4행정, DOHC) 엔진을 이 저장소의 Speeduino
포크 펌웨어로 제어할 때 필요한 설정과 미확정 항목을 정리한다.

## 하드웨어

- 보드: 시판 Speeduino v0.4.x (Mega2560 기반)
- 크랭크/캠 센서: CBR300R 순정 센서 사용 예정
- PlatformIO 빌드 환경: `env:megaatmega2560` (기본 인젝터 4채널/점화 5채널 —
  단기통은 채널 1개만 사용하므로 별도 `-6-3`, `-8-1` 변형은 불필요)

## 점화/분사 로직: 코드 변경 불필요

Speeduino는 `nCylinders == 1`을 이미 명시적으로 지원한다.

- `speeduino/scheduler_ignition_controller.cpp` `initScheduleAngles()`의
  `case 1:` — 기본(non-sequential) 모드에서는 `CRANK_ANGLE_MAX_IGN = 360`으로
  고정되어, **캠 동기화 없이 크랭크 1회전마다 점화**한다. `sparkMode`를
  `IGN_MODE_SEQUENTIAL`로 바꾸고 4행정으로 설정한 경우에만 720도(2회전) 주기로
  전환된다.
- `speeduino/scheduler_fuel_controller.cpp` `initFuelScheduleAngles()`의
  `case 1:` — 마찬가지로 non-sequential(`injLayout` != `INJ_SEQUENTIAL`)일 때
  회전마다 분사되는 `nSquirts` 계산이 이미 처리되어 있다.

즉 "매 회전 점화/분사"는 신규 코드가 아니라 **TunerStudio 설정만으로** 얻을 수
있는 기본 동작이다. 아래 설정값 사용:

| 설정 | 값 | 비고 |
|---|---|---|
| nCylinders | 1 | |
| strokes | 4-stroke | |
| sparkMode | 기본(non-sequential, wasted 방식) | 캠 센서 미필요 |
| injLayout | Batch / Simultaneous (non-sequential) | 캠 센서 미필요 |

시퀀셜(캠 동기화) 방식으로 전환하고 싶다면 캠 신호가 필요하고, 720도 주기로
동작이 바뀐다. 초기 셋업은 non-sequential로 진행하는 것을 권장.

## 미확정: 트리거 디코더 (블로킹)

CBR300R 순정 크랭크/캠 센서의 정확한 릴럭턴트 이빨 수 / 결치 패턴 / 캠 신호
유무는 공개 자료로 확인하지 못했다. Honda가 서비스 매뉴얼 외에는 공개하지 않는
정보라, 아래 중 하나로 확정해야 한다.

1. CBR300R 서비스 매뉴얼의 점화 시스템(CKP 센서) 파형/사양 확인
2. 실측: 배선 후 Speeduino의 Tooth Logger / Composite Logger로 크랭크 신호를
   기록해 이빨 수와 결치 위치를 역산

참고: Speeduino는 이미 단기통 오프로드 바이크용 `DRZ400` 디코더
(`speeduino/decoders.h`, `decoder_init.cpp`)를 내장하고 있다. CBR300R과 동일한
패턴인지는 검증되지 않았으므로, 실측 없이 이 디코더를 그대로 신뢰해 점화
타이밍에 사용해서는 안 된다.

트리거 데이터가 확보되면:

- 기존 디코더(`missingTooth`, `DualWheel`, `DRZ400` 등)로 매칭되는지 확인
- 매칭되는 게 없으면 `speeduino/decoders.cpp` / `decoder_init.cpp`에 신규
  디코더를 추가하는 코드 작업 진행

## 이미 내장된 차량용 부가 기능

아래는 이미 코드로 구현되어 있어 TunerStudio 설정/핀 매핑만 하면 된다.

- `speeduino/src/controllers/boost/` — 부스트 컨트롤
- `speeduino/src/controllers/launch/` — 런치 컨트롤
- `speeduino/src/controllers/fan/` — 냉각팬 제어
- `speeduino/src/controllers/fuelPump/` — 연료펌프 로직
- `speeduino/idle.cpp` — 아이들 제어
- `speeduino/src/controllers/aircon/`, `nitrous/`, `tsCommand/`

## 다음 단계

1. CBR300R 크랭크/캠 센서 트리거 데이터 확보 (서비스 매뉴얼 또는 실측)
2. 확보되면 기존 디코더 매칭 여부 판단 → 필요시 신규 디코더 코드 추가
3. TunerStudio 튠 파일(.msq) 초안 작성 (트리거 확정 후)
