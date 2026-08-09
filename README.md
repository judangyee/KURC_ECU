# KURC_ECU

자작 차량용 ECU 제작 프로젝트.

[Speeduino](https://github.com/speeduino/speeduino)를 베이스로 포크하여, 우리 차량/하드웨어 요구사항에 맞게 펌웨어를 커스터마이징합니다.

## Upstream

- 원본 프로젝트: https://github.com/speeduino/speeduino (GPLv2)
- 포크 기준 커밋: `66aabcc72419edf24e884e181f7b17ece29f3a4d` (2026-08-05)
- 라이선스: 원본과 동일하게 [GPLv2](LICENSE)를 유지합니다. 이 프로젝트의 수정 사항도 GPLv2 하에 배포됩니다.

## 대상 차량/엔진

- 엔진: Honda CBR300R (286cc, 단기통, 4행정)
- 보드: 시판 Speeduino v0.4.x (Mega2560 기반)
- 상세 설정/미확정 항목: [docs/cbr300r-setup.md](docs/cbr300r-setup.md)

## 문서

- Speeduino 공식 매뉴얼: https://wiki.speeduino.com (하드웨어 배선, 튜닝, 센서 스펙 등 기본 구조는 그대로 참고 가능)
- [CBR300R 적용 노트](docs/cbr300r-setup.md)

## 빌드

이 저장소는 Speeduino와 동일하게 [PlatformIO](https://platformio.org/) 기반으로 빌드합니다.

```bash
pio run -e megaatmega2560
```

지원 보드/환경 목록은 `platformio.ini`를 참고하세요.

## 디렉터리 구조

- `speeduino/` — ECU 펌웨어 소스 (C++)
- `boards/` — 커스텀 보드 정의 (링커 스크립트 등)
- `lib/` — 외부/커스텀 라이브러리
- `test/` — 유닛 테스트
- `reference/` — 참고 자료

## 커스터마이징 로드맵

- [x] 푸시-투-스타트(스타터 버튼) 컨트롤 추가
- [x] 안 쓰는 기능 제거 (나이트로스, WMI, VVT, 로터리 점화, 스테이지드 인젝션,
      시퀀셜 연료 트림, CAN/세컨더리 시리얼) — 상세: [docs/cbr300r-setup.md](docs/cbr300r-setup.md)
- [ ] CBR300R 크랭크/캠 트리거 데이터 확보 (서비스 매뉴얼 또는 Tooth Logger 실측)
- [ ] 트리거 디코더 확정 (기존 디코더 매칭 또는 신규 추가)
- [ ] TunerStudio 튠(.msq) 초안 작성

## Contributors

이 프로젝트는 [Speeduino](https://github.com/speeduino/speeduino) 커뮤니티의 코드를 기반으로 합니다. 원본 기여자 목록은 upstream 저장소를 참고하세요.
