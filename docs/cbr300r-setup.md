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

### 순정 센서 사용 시 실측/확인해야 할 값

#### A. 크랭크(1차) 트리거 신호

1. **회전당 총 펄스(이빨) 수** — 전기적으로 몇 개의 신호가 찍히는지
2. **결치(gap) 크기** — 몇 개 이빨이 비어있는 구간이 있는지 (동기화 기준점)
3. **신호 종류** — VR(가변자기저항, 2선, RPM에 따라 진폭이 변하는 AC 파형) vs
   홀/광학(3선, 고정 진폭 디지털 사각파, 5V 전원 필요). 배선 방식이 완전히
   달라지므로 반드시 확인
4. **엣지 극성** — 이빨 신호가 rising에서 잡히는지 falling에서 잡히는지
5. **결치 기준 → 실제 크랭크 TDC까지의 각도** — `triggerAngle` 값이 됨.
   크랭크 풀리/플라이휠에 TDC 타이밍 마크가 있어야 정확히 측정 가능
6. (VR인 경우) **저RPM(크랭킹, 약 150~300rpm)에서의 신호 진폭** — 너무 약하면
   시동 크랭킹 시 신호를 못 읽을 수 있음

#### B. 캠(2차) 신호 — 시퀀셜을 안 쓰면 측정 불필요

- 캠 1회전당 펄스 수, 엣지 극성, 신호 종류
- 크랭크 결치 대비 캠 펄스가 어느 위치에서 발생하는지 (위상)

#### C. 전기적/배선 스펙

- 커넥터 핀아웃 (신호선/접지/센서 전원선 구분)
- 홀센서라면 필요 전원 전압 (5V vs 12V — CBR300R은 최신 PGM-FI라 5V일
  가능성이 높지만 확인 필요)
- 접지가 엔진블록 공용인지 ECU 전용 신호접지인지

#### D. 측정 방법

- 커넥터 핀아웃은 서비스 매뉴얼 배선도 또는 멀티미터 통전 체크로 확인
- 파형은 오실로스코프가 정확하지만, 없으면 Speeduino 보드에 임시로 연결해서
  Tooth Logger / Composite Logger로 캡처 가능
- 크랭킹 RPM과 아이들 RPM 둘 다 캡처 권장 (VR 신호는 RPM에 따라 파형이
  달라짐)

측정한 값은 `config_pages.h`의 다음 설정 필드로 그대로 매핑된다:
`TrigPattern`, `triggerTeeth`, `triggerMissingTeeth`, `triggerAngle`,
`TrigEdge`, `TrigSpeed`(크랭크 기준 vs 캠 기준), `trigPatternSec`(2차 트리거용).

## 이미 내장된 차량용 부가 기능

아래는 이미 코드로 구현되어 있어 TunerStudio 설정/핀 매핑만 하면 된다.

- `speeduino/src/controllers/boost/` — 부스트 컨트롤
- `speeduino/src/controllers/launch/` — 런치 컨트롤
- `speeduino/src/controllers/fan/` — 냉각팬 제어
- `speeduino/src/controllers/fuelPump/` — 연료펌프 로직
- `speeduino/idle.cpp` — 아이들 제어
- `speeduino/src/controllers/aircon/`, `tsCommand/`

## 신규 추가: 푸시-투-스타트(스타터 버튼) 컨트롤

stock Speeduino에는 없던 기능이라 새로 구현했다. `speeduino/src/controllers/starter/`.

- 버튼을 짧게 눌렀다 떼도(rising edge) 스타터 릴레이 출력이 래치(latch)되어 유지된다.
- `currentStatus.rotationStatus`가 `Running`으로 바뀌면(엔진 시동 성공) 자동으로 릴레이를 끈다 —
  기존 크랭킹 RPM 임계값(`configPage4.crankRPM`)을 그대로 재사용하므로 별도 RPM 설정을 추가하지 않았다.
- `configPage15.starterMaxCrankTime`(0.1초 단위) 시간을 넘기면 안전상 자동으로 끈다 (시동이 안 걸릴 때 스타터 모터 보호).
- 크랭킹 도중 버튼을 다시 누르면 수동 취소된다.

TunerStudio에서 `&Accessories → Push Button Start` 메뉴로 활성화/핀/극성/최대 크랭킹 시간을 설정한다.
`reference/speeduino.ini`의 config15 페이지 106~108바이트에 필드를 추가했다 (기존 예약 공간을 사용해
페이지 전체 크기(256바이트)는 그대로 유지됨).

**빌드 검증**: PlatformIO의 패키지 레지스트리(platformio.org)는 이 세션 네트워크 정책상 막혀 있어
`pio run`은 사용하지 못했다. 대신 `apt`로 `gcc-avr`/`avr-libc`/`arduino-core-avr`를 설치하고,
`platformio.ini`의 `env:megaatmega2560` 빌드 플래그를 그대로 재현해 전체 소스를 수동으로
컴파일·링크했다. 매 기능 제거/추가마다 이 방식으로 재검증했으며, 최종적으로 에러 없이
`firmware.elf`가 생성됨을 확인했다. 다만 PlatformIO가 쓰는 정확한 링커 스크립트/최적화 플래그와는
완전히 동일하지 않으므로, 실제 보드에 플래싱하기 전에는 가능하면 `pio run -e megaatmega2560`으로
한 번 더 확인하는 것을 권장한다.

## 사용하지 않는 기능 제거

단기통 자작차 용도로 아래 기능을 펌웨어에서 제거했다 (2026-08-09 기준):

- 나이트로스(N2O) 제어
- WMI (워터메탄 분사) 제어
- VVT (가변밸브타이밍) 제어
- 로터리 점화 모드 (RX-8류 로터리 엔진 전용)
- 스테이지드 인젝션 (2단 인젝터)
- 시퀀셜 연료 트림 (실린더별 보정)
- CAN 버스(Mega2560엔 애초에 하드웨어가 없어 컴파일도 안 됐음) / 세컨더리 시리얼(외부 CAN·시리얼 릴레이)

**제거 원칙**: 실제 제어 로직(연산, 출력 구동)은 걷어내되, `config_pages.h`의 EEPROM 페이지 구조나
`logger.cpp`의 고정 바이트 오프라인 로그 프로토콜, `pages.cpp`/`storage.cpp`의 테이블 저장 슬롯은
건드리지 않았다. 이유는 이런 것들을 재배치하면 `reference/speeduino.ini`의 오프셋도 전부 같이
맞춰야 해서 위험도가 훨씬 커지기 때문. 남은 필드/테이블은 그냥 값이 안 쓰이는 채로 존재만 한다
(플래시 비용은 미미함). TunerStudio 메뉴에서는 해당 기능들이 안 보이도록 정리했다.

로터리 점화의 "Rotary Ignition" 서브메뉴처럼 `{ sparkMode == 4 }` 같은 조건부 메뉴는 실수로 값을
Rotary로 설정하면 다시 나타날 수 있다 — 코드상으로는 wasted spark로 안전하게 폴백하므로 동작에는
문제없지만, 완전히 숨기려면 ini 드롭다운 옵션 자체를 손봐야 한다 (미반영, 낮은 우선순위로 남겨둠).

빌드 크기 변화 (수동 컴파일 기준, `env:megaatmega2560` 빌드 플래그):

| 시점 | text (bytes) |
|---|---|
| 임포트 직후 + 스타터 버튼 추가 후 | 260,400 |
| 나이트로스 제거 | 259,320 |
| WMI + VVT 제거 | 252,566 |
| 로터리 점화 제거 | 251,498 |
| 스테이지드 인젝션 제거 | 250,470 |
| 시퀀셜 연료 트림 제거 | 248,586 |
| CAN / 세컨더리 시리얼 제거 | 247,308 |

총 약 13KB 절감 (Mega2560 256KB 플래시 기준).

## 다음 단계

1. CBR300R 크랭크/캠 센서 트리거 데이터 확보 (서비스 매뉴얼 또는 실측)
2. 확보되면 기존 디코더 매칭 여부 판단 → 필요시 신규 디코더 코드 추가
3. TunerStudio 튠 파일(.msq) 초안 작성 (트리거 확정 후)
