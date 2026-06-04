# ac_bam (ArchiCAD BIM Automation Module)

**ac_bam**은 ArchiCAD 사용자의 생산성을 극대화하기 위해 설계된 프리미엄 자동화 도구 모음입니다.  
**ac_bam** is a premium automation toolkit designed to maximize the productivity of ArchiCAD users.

엑셀 연동, 면적 기반 형상 변경, 배치 라벨링 등 반복적인 작업을 단 몇 번의 클릭으로 해결합니다.  
It solves repetitive tasks such as Excel integration, area-based reshaping, and batch labeling with just a few clicks.

---

## 🚀 주요 기능 (Key Features)

### 1. Excel to Zone (엑셀 기반 실 생성)
- **사용법**: 엑셀 파일(.xlsx)을 선택하고 `Auto Create` 버튼을 클릭합니다.  
  **How to use**: Select an Excel file (.xlsx) and click the `Auto Create` button.
- **기능**: 엑셀에 정의된 층별 실 이름과 데이터를 바탕으로 ArchiCAD 내에 자동으로 Zone 요소를 배치합니다.  
  **Function**: Automatically places Zone elements in ArchiCAD based on room names and data defined by story in the Excel file.

### 2. Calculate Area & Reshape (면적 계산 및 변형)
- **Target Area**: 목표 면적(m²)을 입력합니다.  
  **Target Area**: Enter the desired area (m²).
- **Tolerance**: 허용 오차 범위를 설정합니다.  
  **Tolerance**: Set the allowable error range.
- **Reshape Zone**: 선택된 Zone이나 Hatch의 위치를 유지하면서 목표 면적에 맞춰 크기를 자동 조정합니다.  
  **Reshape Zone**: Automatically adjusts the size of selected Zones or Hatches to match the target area while maintaining their position.
  - **Fix Width (X)**: 가로 길이를 유지하고 세로 길이를 조정하여 면적을 맞춥니다.  
    **Fix Width (X)**: Maintains the width and adjusts the height to match the area.
  - **Fix Height (Y)**: 세로 길이를 유지하고 가로 길이를 조정하여 면적을 맞춥니다.  
    **Fix Height (Y)**: Maintains the height and adjusts the width to match the area.
- **Sum Selected**: 선택된 여러 요소의 전체 면적 합계를 즉시 확인합니다.  
  **Sum Selected**: Instantly check the total area sum of multiple selected elements.

### 3. Magic Wand Creation (배치 자동 생성)
- **대상**: Zone, Mesh, Morph  
  **Targets**: Zone, Mesh, Morph
- **기능**: 선택된 폴리라인이나 폐쇄된 영역을 소스로 삼아, 매직완드 알고리즘을 통해 여러 요소를 한 번에 일괄 생성합니다.  
  **Function**: Batch creates multiple elements from selected polylines or closed areas using the Magic Wand algorithm.
- **H(mm)**: 생성될 요소의 높이 값을 설정할 수 있습니다.  
  **H(mm)**: You can set the height value for the generated elements.

### 4. Labeling (자동 라벨링 도구)
- **Prefix**: 라벨 앞에 붙을 접두어를 입력합니다.  
  **Prefix**: Enter the prefix for the label.
- **Pick**: 기존 요소에서 텍스트 정보를 가져옵니다.  
  **Pick**: Extract text information from an existing element.
- **Style**: 라벨링의 스타일(접두어+번호 등)을 선택합니다.  
  **Style**: Select the labeling style (e.g., Prefix + Number).
- **Start**: 시작 번호를 설정합니다.  
  **Start**: Set the starting index.
- **Apply Labeling**: 선택된 요소들에 규칙에 따른 라벨을 순차적으로 적용합니다.  
  **Apply Labeling**: Sequentially applies labels to selected elements based on the rules.


### 6. 3D Resolution Helper (3D 해상도 도우미)
- **기능**: 3D 창의 해상도를 특정 값이나 비율로 조정합니다.  
  **Function**: Adjusts the resolution of the 3D window to a specific value or ratio.
- **Lock Ratio**: 가로/세로 비율을 고정하여 해상도를 변경합니다.  
  **Lock Ratio**: Changes resolution while maintaining the aspect ratio.
- **Match Drawing**: 도면 영역 크기에 맞춰 3D 창 크기를 동기화합니다.  
  **Match Drawing**: Synchronizes the 3D window size with the drawing area size.

### 6. Ground Plane Generator (지표면 생성기)
- **기능**: 선택된 여러 모프(Morph) 요소의 가중평균 높이를 계산하여, 대지를 덮는 넓은 지표면 모프를 자동 생성합니다.  
  **Function**: Calculates the weighted average height of selected Morph elements and automatically generates a large ground plane Morph.

---

## 📅 업데이트 이력 (Release Notes)

### v1.0.1 (2026-06-04)
- **Magic Wand 생성 오류 완벽 수정**: Zone 및 Mesh 생성 시 `nSubPolys` 파라미터가 0으로 복사되어 발생하던 `APIERR_BADPARS (-2130313111)` 오류 해결. 이제 폴리라인 소스를 바탕으로 한 번에 면 기반 요소 생성이 정상적으로 작동합니다. (Fixed polygon parameter bug during Magic Wand creation)
- **Terrain 패널 UI 간소화**: 사용자 피드백을 반영하여 Terrain 패널 하단의 'Polygon Point Reduction' 관련 옵션을 제거하여 패널을 직관적으로 개선했습니다. (Removed Simplify Poly UI from Terrain Palette)
- **멀티 버전 대응**: ArchiCAD 27 및 29 환경 모두에서 동일한 기능을 사용할 수 있도록 코드를 최적화하고 배포 스크립트를 통합했습니다.

### v1.0.0 (2026-05-06)
- **Ground Plane Generator 안정성 강화**: 층 정보 참조 시 발생하던 메모리 크래시 완전 해결 (Fixed story data memory crash)
- **자동 층 상속 및 1.0m 오프셋**: 새 지표면이 원본 영역을 완전히 덮을 수 있도록 자동 확장 기능 추가

---

## 📄 라이선스 및 지원 (License & Support)
본 애드온은 프리미엄 도구로 제공됩니다. 라이선스 키 발급을 원하시면 아래 메일 주소로 연락해 주시기 바랍니다.  
This add-on is provided as a premium tool. If you wish to obtain a license key, please contact us at the email address below.

- **Email**: [p.juhyung@daum.net](mailto:p.juhyung@daum.net)
- **문의 내용**: 라이선스 키 요청 및 기술 지원  
  **Inquiry**: License key requests and technical support.
