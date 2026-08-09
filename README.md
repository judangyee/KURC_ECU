# KURC_ECU

자작 차량용 ECU 제작 프로젝트.

[Speeduino](https://github.com/speeduino/speeduino)를 베이스로 포크하여, 우리 차량/하드웨어 요구사항에 맞게 펌웨어를 커스터마이징합니다.

## Upstream

- 원본 프로젝트: https://github.com/speeduino/speeduino (GPLv2)
- 포크 기준 커밋: `66aabcc72419edf24e884e181f7b17ece29f3a4d` (2026-08-05)
- 라이선스: 원본과 동일하게 [GPLv2](LICENSE)를 유지합니다. 이 프로젝트의 수정 사항도 GPLv2 하에 배포됩니다.

## 문서

- Speeduino 공식 매뉴얼: https://wiki.speeduino.com (하드웨어 배선, 튜닝, 센서 스펙 등 기본 구조는 그대로 참고 가능)

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

- [ ] 우리 차량 하드웨어(센서/인젝터/점화 채널 구성)에 맞는 보드 정의 추가
- [ ] 필요 없는 기능/보드 타겟 정리
- [ ] 우리 배선/핀맵 문서화

## Contributors

이 프로젝트는 [Speeduino](https://github.com/speeduino/speeduino) 커뮤니티의 코드를 기반으로 합니다. 원본 기여자 목록은 upstream 저장소를 참고하세요.
