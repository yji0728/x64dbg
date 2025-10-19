## 가상화 기반 패커의 메모리 레벨 OEP 탐지 방법 연구

### 1. 가상화 패커의 실행 메커니즘 분석

#### 1.1 가상화 패커의 한계점
```
[VM 바이트코드] → [VM 핸들러] → [실제 CPU 명령어] → [실행]
                                    ↑
                              이 시점에서 탐지 가능
```

가상화 패커도 결국 다음과 같은 한계를 가집니다:
- **최종 실행은 실제 CPU 명령어**: VM이 아무리 복잡해도 결국 실제 CPU가 이해할 수 있는 명령어로 변환되어야 함
- **API 호출 필수**: Windows API는 VM 코드가 아닌 실제 주소를 통해 호출되어야 함
- **메모리 접근 패턴**: 원본 프로그램의 데이터 구조와 메모리 접근 패턴은 유지됨

### 2. 고급 OEP 탐지 전략

#### 2.1 JIT(Just-In-Time) 코드 생성 추적
```cpp
class JITCodeTracker {
private:
    std::map<DWORD_PTR, CodeBlock> jit_blocks;
    std::set<DWORD_PTR> execution_flow;
    
public:
    void OnMemoryAllocation(DWORD_PTR address, SIZE_T size, DWORD protect) {
        if (protect & PAGE_EXECUTE_READWRITE || protect & PAGE_EXECUTE_READ) {
            // 실행 가능한 메모리 할당 감지
            MonitorMemoryBlock(address, size);
        }
    }
    
    void OnCodeExecution(DWORD_PTR address) {
        // 새로운 메모리 영역에서 코드 실행 감지
        if (!IsKnownVMHandler(address)) {
            // VM 핸들러가 아닌 새로운 코드 실행
            AnalyzePotentialOEP(address);
        }
    }
};
```

#### 2.2 마이크로 실행 패턴 분석 (Micro-Execution Pattern Analysis)
```cpp
struct ExecutionPattern {
    DWORD_PTR address;
    std::vector<BYTE> instructions;
    std::vector<DWORD_PTR> api_calls;
    DWORD execution_count;
    double entropy_change;
};

class MicroPatternAnalyzer {
public:
    void AnalyzeExecutionTrace() {
        // 짧은 시간 동안의 실행 패턴 수집
        std::vector<ExecutionPattern> patterns;
        
        // VM 핸들러의 반복적 패턴 vs 실제 애플리케이션 코드 구분
        for (auto& pattern : patterns) {
            if (IsApplicationPattern(pattern)) {
                // 높은 API 호출 밀도
                // 낮은 반복성
                // 의미있는 제어 흐름
                MarkAsPotentialOEP(pattern.address);
            }
        }
    }
    
private:
    bool IsApplicationPattern(const ExecutionPattern& pattern) {
        // API 호출 빈도가 높음
        if (pattern.api_calls.size() > threshold_api_calls) return true;
        
        // VM 핸들러와 다른 실행 패턴
        if (!MatchesVMHandlerSignature(pattern)) return true;
        
        // 스택 프레임 설정 패턴 감지
        if (DetectsFunctionPrologue(pattern)) return true;
        
        return false;
    }
};
```

### 3. 계층적 메모리 추상화 분석

#### 3.1 메모리 접근 그래프 구축
```cpp
class MemoryAccessGraph {
private:
    struct MemoryRegion {
        DWORD_PTR base;
        SIZE_T size;
        std::set<DWORD_PTR> accessors;  // 이 영역에 접근한 코드 주소
        std::map<DWORD_PTR, AccessType> access_patterns;
    };
    
public:
    void BuildAccessGraph() {
        // 메모리 접근 패턴을 그래프로 구축
        // VM 핸들러: 제한된 메모리 영역에 반복 접근
        // 실제 코드: 다양한 메모리 영역에 의미있는 패턴으로 접근
    }
    
    DWORD_PTR FindTransitionPoint() {
        // VM 영역에서 애플리케이션 영역으로 전환되는 지점 탐지
        for (auto& region : memory_regions) {
            if (IsTransitionFromVMToApp(region)) {
                return region.first_app_access;
            }
        }
    }
};
```

### 4. API 호출 체인 재구성

#### 4.1 간접 API 호출 추적
```cpp
class IndirectAPITracker {
private:
    struct APICallContext {
        DWORD_PTR call_site;      // API를 호출한 위치
        DWORD_PTR return_address; // 리턴 주소
        std::string api_name;
        std::vector<DWORD_PTR> parameters;
        DWORD_PTR stack_frame;
    };
    
public:
    void TrackAPICall(DWORD_PTR address) {
        // VM이 API를 호출할 때의 컨텍스트 수집
        APICallContext context = CaptureContext();
        
        // 실제 애플리케이션 코드의 리턴 주소 추적
        DWORD_PTR real_caller = FindRealCaller(context);
        
        if (!IsVMHandler(real_caller)) {
            // VM 핸들러가 아닌 곳으로 리턴 = OEP 후보
            potential_oep_addresses.insert(real_caller);
        }
    }
    
private:
    DWORD_PTR FindRealCaller(const APICallContext& context) {
        // 스택을 역추적하여 VM 레이어를 건너뛴 실제 호출자 찾기
        std::vector<DWORD_PTR> call_stack = UnwindStack(context.stack_frame);
        
        for (auto addr : call_stack) {
            if (!IsInVMSpace(addr) && IsExecutableCode(addr)) {
                return addr;
            }
        }
        return 0;
    }
};
```

### 5. 동적 코드 재구성 탐지

#### 5.1 코드 캐시 분석
```cpp
class CodeCacheAnalyzer {
private:
    struct CodeCache {
        DWORD_PTR address;
        std::vector<BYTE> code;
        DWORD execution_count;
        std::set<DWORD_PTR> jump_targets;
    };
    
public:
    void AnalyzeDynamicCode() {
        // VM이 생성한 코드 조각들을 수집
        std::map<DWORD_PTR, CodeCache> code_caches;
        
        // 코드 조각들 간의 제어 흐름 분석
        for (auto& cache : code_caches) {
            if (HasCompleteControlFlow(cache.second)) {
                // 완전한 함수 형태를 갖춘 코드 발견
                AnalyzeAsOEPCandidate(cache.first);
            }
        }
    }
    
    bool HasCompleteControlFlow(const CodeCache& cache) {
        // 함수 프롤로그/에필로그 존재
        // 의미있는 제어 흐름 구조
        // API 호출 패턴
        return DetectFunctionStructure(cache);
    }
};
```

### 6. 하이브리드 탐지 알고리즘

#### 6.1 다층 분석 프레임워크
```cpp
class HybridOEPDetector {
private:
    JITCodeTracker jit_tracker;
    MicroPatternAnalyzer pattern_analyzer;
    MemoryAccessGraph memory_graph;
    IndirectAPITracker api_tracker;
    CodeCacheAnalyzer cache_analyzer;
    
public:
    DWORD_PTR DetectOEP() {
        // 1단계: VM 실행 영역 식별
        IdentifyVMSpace();
        
        // 2단계: 병렬 분석 시작
        std::thread t1([this]() { jit_tracker.StartTracking(); });
        std::thread t2([this]() { pattern_analyzer.StartAnalysis(); });
        std::thread t3([this]() { api_tracker.StartTracking(); });
        
        // 3단계: 실행 진행
        RunUntilStableState();
        
        // 4단계: 결과 종합
        auto candidates = CollectCandidates();
        
        // 5단계: 점수 기반 평가
        return EvaluateCandidates(candidates);
    }
    
private:
    DWORD_PTR EvaluateCandidates(const std::vector<OEPCandidate>& candidates) {
        for (auto& candidate : candidates) {
            int score = 0;
            
            // API 호출 밀도
            score += CalculateAPICallDensity(candidate) * 30;
            
            // VM 영역과의 거리
            score += CalculateDistanceFromVM(candidate) * 20;
            
            // 코드 구조 완성도
            score += AnalyzeCodeStructure(candidate) * 25;
            
            // 메모리 접근 패턴
            score += AnalyzeMemoryPattern(candidate) * 25;
            
            candidate.score = score;
        }
        
        // 가장 높은 점수의 후보 반환
        return GetHighestScoreCandidate(candidates);
    }
};
```

### 7. 실험적 접근: 시맨틱 실행 추적

#### 7.1 의미론적 명령어 그룹화
```cpp
class SemanticExecutionTracer {
private:
    enum SemanticOperation {
        MEMORY_ALLOCATION,
        FILE_OPERATION,
        REGISTRY_ACCESS,
        NETWORK_OPERATION,
        PROCESS_CREATION,
        UI_OPERATION
    };
    
public:
    void TraceSemanticOperations() {
        // VM 레이어를 통과하더라도 의미론적 작업은 유지됨
        std::vector<SemanticOperation> operations;
        
        // 일반적인 프로그램 시작 패턴 감지
        if (DetectsProgramInitPattern(operations)) {
            // 메모리 할당 → 설정 로드 → UI 초기화 등
            MarkTransitionPoint();
        }
    }
};
```

### 8. 실전 적용 예제

#### 8.1 VMProtect 3.x 대응
```cpp
void HandleVMProtect3() {
    // VMProtect의 특징적인 VM 진입 패턴 식별
    auto vm_entry = FindVMEntry();
    
    // VM 핸들러 테이블 위치 파악
    auto handler_table = LocateHandlerTable(vm_entry);
    
    // 핸들러 실행 추적하며 실제 작업 분리
    while (IsInVM()) {
        auto current_handler = GetCurrentHandler();
        
        // VMEXIT 명령어 패턴 감지
        if (IsVMExitPattern(current_handler)) {
            // VM 탈출 지점 = 잠재적 OEP
            auto exit_point = GetVMExitTarget();
            VerifyAsOEP(exit_point);
        }
    }
}
```

### 9. 구현 시 주의사항

1. **성능 최적화**: 모든 명령어를 추적하면 속도가 매우 느려지므로, 샘플링과 휴리스틱을 적절히 조합
2. **오탐 방지**: VM 핸들러 자체를 OEP로 오인하지 않도록 충분한 검증 단계 필요
3. **메모리 관리**: 대량의 실행 추적 데이터를 효율적으로 관리
4. **안정성**: VM 실행 중 개입으로 인한 크래시 방지

이러한 방법들을 조합하면 가상화 기반 패커에서도 높은 확률로 실제 OEP를 찾을 수 있습니다. 핵심은 VM 레이어의 특성을 이해하고, 그것과 실제 애플리케이션 코드를 구분하는 다양한 지표를 활용하는 것입니다.
