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

---

## 고급 Themida 급 가상화 언패킹 기술 연구

### 10. VM 컨텍스트 돌연변이 추적 (VM Context Mutation Tracking)

Themida와 같은 고급 가상화 보호는 VM 컨텍스트를 지속적으로 변조하여 분석을 어렵게 합니다.

#### 10.1 동적 레지스터 매핑 추적
```cpp
class VMContextMutationTracker {
private:
    struct ContextSnapshot {
        std::map<std::string, DWORD_PTR> vm_registers;  // 가상 레지스터 상태
        std::map<std::string, DWORD_PTR> cpu_registers; // 실제 CPU 레지스터
        DWORD_PTR instruction_pointer;
        DWORD64 timestamp;
        std::vector<BYTE> vm_stack;
    };
    
    std::vector<ContextSnapshot> context_history;
    std::map<DWORD_PTR, std::string> register_mappings;
    
public:
    void TrackContextMutation() {
        // VM이 레지스터 매핑을 변경할 때마다 추적
        for (size_t i = 1; i < context_history.size(); i++) {
            auto& prev = context_history[i - 1];
            auto& curr = context_history[i];
            
            // 레지스터 매핑 변화 감지
            auto mapping_changes = DetectMappingChanges(prev, curr);
            
            if (!mapping_changes.empty()) {
                // 매핑이 변경된 시점 기록
                AnalyzeMutationPattern(mapping_changes);
            }
        }
    }
    
    // VM 레지스터의 실제 값 복원
    DWORD_PTR ReconstructActualValue(const std::string& vm_register, 
                                      DWORD_PTR vm_context_address) {
        // 현재 매핑 정보를 사용하여 실제 값 계산
        auto current_mapping = GetCurrentMapping(vm_register);
        return ResolveValueThroughMapping(current_mapping, vm_context_address);
    }
    
private:
    std::vector<MappingChange> DetectMappingChanges(
        const ContextSnapshot& prev, 
        const ContextSnapshot& curr) {
        std::vector<MappingChange> changes;
        
        // 가상 레지스터 매핑 비교
        for (auto& [vm_reg, vm_val] : curr.vm_registers) {
            if (prev.vm_registers.count(vm_reg)) {
                auto prev_cpu = FindCPUMapping(prev, vm_reg);
                auto curr_cpu = FindCPUMapping(curr, vm_reg);
                
                if (prev_cpu != curr_cpu) {
                    changes.push_back({vm_reg, prev_cpu, curr_cpu});
                }
            }
        }
        return changes;
    }
};
```

### 11. 제어 흐름 난독화 분석 (Control Flow Obfuscation Analysis)

Themida는 복잡한 제어 흐름 난독화를 사용합니다.

#### 11.1 불투명 술어 탐지 (Opaque Predicate Detection)
```cpp
class OpaquePredicateDetector {
private:
    struct PredicateInfo {
        DWORD_PTR address;
        std::string condition;      // 조건식
        bool is_always_true;        // 항상 참인지
        bool is_always_false;       // 항상 거짓인지
        int evaluation_count;       // 평가 횟수
        std::set<bool> results;     // 실제 결과 집합
    };
    
public:
    void AnalyzeControlFlow() {
        std::map<DWORD_PTR, PredicateInfo> predicates;
        
        // 조건 분기 수집 및 분석
        for (auto& pred : predicates) {
            // 여러 번 실행되었지만 항상 같은 결과
            if (pred.second.evaluation_count > 10 && 
                pred.second.results.size() == 1) {
                
                // 불투명 술어로 판정
                MarkAsOpaquePredicate(pred.first);
                
                // 실제로 실행되는 경로만 추적
                SimplifyControlFlow(pred.first, *pred.second.results.begin());
            }
        }
    }
    
    // 가짜 제어 흐름 제거
    void SimplifyControlFlow(DWORD_PTR predicate_addr, bool taken_branch) {
        auto basic_block = GetBasicBlock(predicate_addr);
        
        // 실행되지 않는 브랜치 제거
        if (taken_branch) {
            RemoveDeadBranch(basic_block.false_branch);
        } else {
            RemoveDeadBranch(basic_block.true_branch);
        }
        
        // 단순화된 제어 흐름으로 재구성
        ReconstructSimplifiedCFG();
    }
};
```

#### 11.2 간접 분기 타겟 해석
```cpp
class IndirectBranchResolver {
private:
    struct BranchTarget {
        DWORD_PTR branch_site;      // 분기 명령어 위치
        std::set<DWORD_PTR> targets; // 가능한 타겟들
        bool is_vm_dispatcher;       // VM 디스패처 여부
        DWORD_PTR actual_target;     // 실제 타겟
    };
    
public:
    void ResolveIndirectBranches() {
        std::map<DWORD_PTR, BranchTarget> branches;
        
        // 간접 분기 수집
        CollectIndirectBranches(branches);
        
        // 각 분기의 실제 타겟 분석
        for (auto& [addr, info] : branches) {
            // 실행 추적으로 실제 타겟 수집
            auto runtime_targets = TraceRuntimeTargets(addr);
            
            if (runtime_targets.size() == 1) {
                // 항상 같은 곳으로 가는 간접 분기
                info.actual_target = *runtime_targets.begin();
                SimplifyToDirectBranch(addr, info.actual_target);
            } else if (IsVMDispatcher(runtime_targets)) {
                // VM 디스패처 패턴 감지
                info.is_vm_dispatcher = true;
                AnalyzeDispatcherTable(addr, runtime_targets);
            }
        }
    }
    
private:
    bool IsVMDispatcher(const std::set<DWORD_PTR>& targets) {
        // VM 핸들러들은 특정 메모리 영역에 집중됨
        if (targets.size() < 10 || targets.size() > 1000) return false;
        
        // 타겟들의 메모리 분포 분석
        auto [min_addr, max_addr] = std::minmax_element(
            targets.begin(), targets.end());
        
        SIZE_T handler_region_size = *max_addr - *min_addr;
        
        // VM 핸들러 테이블의 특징: 밀집된 함수 포인터 집합
        return handler_region_size < 0x100000 && // 1MB 이내
               targets.size() > 20;                // 충분한 핸들러 수
    }
};
```

### 12. 하드웨어 브레이크포인트 회피 탐지

Themida는 하드웨어 브레이크포인트와 디버거를 적극적으로 탐지합니다.

#### 12.1 디버그 레지스터 접근 모니터링
```cpp
class DebugRegisterMonitor {
private:
    struct DRAccessInfo {
        DWORD_PTR access_point;
        std::string register_name;  // DR0, DR1, DR2, DR3, DR6, DR7
        bool is_read;
        bool is_write;
        DWORD_PTR value;
        DWORD64 timestamp;
    };
    
    std::vector<DRAccessInfo> dr_access_log;
    
public:
    void MonitorDebugRegisters() {
        // 디버그 레지스터 접근 시도 감지
        SetupDebugRegisterHooks();
        
        // VM이 디버그 레지스터를 확인하는 패턴 분석
        while (IsInVM()) {
            auto access = WaitForDRAccess();
            dr_access_log.push_back(access);
            
            if (IsAntiDebugCheck(access)) {
                // 안티 디버그 체크 우회
                SpoofDebugRegisterValue(access);
            }
        }
    }
    
private:
    bool IsAntiDebugCheck(const DRAccessInfo& access) {
        // DR7 읽기는 일반적으로 브레이크포인트 체크
        if (access.register_name == "DR7" && access.is_read) {
            return true;
        }
        
        // DR0-DR3 체크는 하드웨어 BP 탐지
        if ((access.register_name == "DR0" || 
             access.register_name == "DR1" ||
             access.register_name == "DR2" || 
             access.register_name == "DR3") && access.is_read) {
            return true;
        }
        
        return false;
    }
    
    void SpoofDebugRegisterValue(const DRAccessInfo& access) {
        // 디버그 레지스터가 비어있는 것처럼 속임
        if (access.register_name.substr(0, 2) == "DR") {
            SetSpoofedValue(access.register_name, 0);
        }
    }
};
```

### 13. VM 핸들러 다형성 분석

고급 가상화는 VM 핸들러를 실행 시마다 다르게 생성합니다.

#### 13.1 다형성 핸들러 정규화
```cpp
class PolymorphicHandlerAnalyzer {
private:
    struct HandlerSignature {
        std::vector<BYTE> semantic_opcodes; // 의미론적 명령어 패턴
        std::set<std::string> api_calls;    // 사용하는 API
        std::vector<int> operand_pattern;   // 피연산자 패턴
        double code_entropy;                 // 코드 엔트로피
    };
    
    std::map<std::string, std::vector<DWORD_PTR>> handler_groups;
    
public:
    void AnalyzePolymorphicHandlers() {
        std::vector<DWORD_PTR> handler_addresses;
        CollectAllHandlers(handler_addresses);
        
        // 각 핸들러를 정규화된 시그니처로 변환
        for (auto addr : handler_addresses) {
            auto signature = ExtractSemanticSignature(addr);
            
            // 유사한 시그니처를 가진 핸들러 그룹화
            std::string sig_hash = ComputeSignatureHash(signature);
            handler_groups[sig_hash].push_back(addr);
        }
        
        // 그룹별로 대표 핸들러 선택
        for (auto& [hash, handlers] : handler_groups) {
            if (handlers.size() > 1) {
                // 다형성 핸들러 그룹 발견
                AnalyzePolymorphicGroup(handlers);
            }
        }
    }
    
private:
    HandlerSignature ExtractSemanticSignature(DWORD_PTR handler_addr) {
        HandlerSignature sig;
        
        // 핸들러 코드 디스어셈블
        auto instructions = Disassemble(handler_addr);
        
        // 실제 의미를 가진 명령어만 추출 (NOP, junk 제외)
        for (auto& inst : instructions) {
            if (!IsJunkInstruction(inst)) {
                // 피연산자는 무시하고 opcode만 저장
                sig.semantic_opcodes.push_back(inst.opcode);
                
                // 피연산자 타입 패턴 (레지스터, 메모리, 즉시값)
                sig.operand_pattern.push_back(
                    ClassifyOperandType(inst.operands));
            }
        }
        
        // API 호출 수집
        sig.api_calls = ExtractAPICalls(instructions);
        
        // 코드 엔트로피 계산
        sig.code_entropy = CalculateEntropy(instructions);
        
        return sig;
    }
    
    void AnalyzePolymorphicGroup(const std::vector<DWORD_PTR>& handlers) {
        // 첫 번째 핸들러를 기준으로 선택
        DWORD_PTR canonical_handler = handlers[0];
        
        // 나머지 핸들러들을 canonical 형태로 매핑
        for (size_t i = 1; i < handlers.size(); i++) {
            MapToCanonicalForm(handlers[i], canonical_handler);
        }
    }
};
```

### 14. 암호화된 바이트코드 스트림 분석

Themida는 VM 바이트코드를 암호화하여 저장합니다.

#### 14.1 바이트코드 복호화 추적
```cpp
class EncryptedBytecodeAnalyzer {
private:
    struct BytecodeStream {
        DWORD_PTR stream_address;
        SIZE_T stream_size;
        std::vector<BYTE> encrypted_data;
        std::vector<BYTE> decrypted_data;
        DWORD_PTR decryption_routine;
        std::vector<BYTE> decryption_key;
    };
    
    std::map<DWORD_PTR, BytecodeStream> bytecode_streams;
    
public:
    void AnalyzeEncryptedBytecode() {
        // VM이 바이트코드를 읽기 전 복호화 루틴 탐지
        LocateDecryptionRoutines();
        
        // 복호화 과정 모니터링
        for (auto& [addr, stream] : bytecode_streams) {
            // 복호화 전/후 데이터 캡처
            CaptureBeforeDecryption(stream);
            
            // 복호화 루틴 실행
            TraceDecryptionRoutine(stream);
            
            // 복호화된 바이트코드 캡처
            CaptureAfterDecryption(stream);
            
            // 복호화 키 추출
            ExtractDecryptionKey(stream);
        }
    }
    
    void DumpDecryptedBytecode() {
        for (auto& [addr, stream] : bytecode_streams) {
            if (!stream.decrypted_data.empty()) {
                // 복호화된 바이트코드를 파일로 덤프
                SaveToFile(stream.stream_address, stream.decrypted_data);
                
                // 바이트코드 디스어셈블 시도
                DisassembleVMBytecode(stream.decrypted_data);
            }
        }
    }
    
private:
    void LocateDecryptionRoutines() {
        // 일반적인 암호화 패턴 탐지
        // 1. XOR 기반 암호화
        // 2. AES/RC4 같은 스트림 암호
        // 3. 커스텀 암호화 알고리즘
        
        auto potential_decryptors = ScanForCryptoPatterns();
        
        for (auto decryptor : potential_decryptors) {
            // 입력/출력 버퍼 추적
            auto io_buffers = TraceIOBuffers(decryptor);
            
            // 버퍼 엔트로피 변화 감지
            if (IsDecryptionRoutine(io_buffers)) {
                RegisterDecryptionRoutine(decryptor);
            }
        }
    }
    
    bool IsDecryptionRoutine(const IOBuffers& buffers) {
        // 입력 버퍼의 엔트로피가 높고 (암호화된 데이터)
        // 출력 버퍼의 엔트로피가 낮으면 (복호화된 코드/데이터)
        double input_entropy = CalculateEntropy(buffers.input);
        double output_entropy = CalculateEntropy(buffers.output);
        
        return input_entropy > 7.0 && output_entropy < 6.0;
    }
};
```

### 15. 멀티 레이어 VM 언팩킹

Themida는 여러 층의 VM을 중첩하여 사용할 수 있습니다.

#### 15.1 VM 레이어 스택 관리
```cpp
class MultiLayerVMUnpacker {
private:
    struct VMLayer {
        int layer_id;
        DWORD_PTR vm_entry;
        DWORD_PTR vm_exit;
        std::set<DWORD_PTR> handlers;
        std::map<DWORD_PTR, std::string> handler_names;
        VMLayer* parent_layer;
        std::vector<VMLayer*> child_layers;
    };
    
    std::stack<VMLayer*> vm_stack;
    VMLayer* current_layer = nullptr;
    
public:
    void UnpackMultiLayerVM() {
        // 최외곽 VM 레이어 진입
        auto root_layer = DetectRootVMLayer();
        vm_stack.push(root_layer);
        current_layer = root_layer;
        
        while (!vm_stack.empty()) {
            auto layer = vm_stack.top();
            
            // 현재 레이어 분석
            AnalyzeVMLayer(layer);
            
            // 중첩된 VM 레이어 탐지
            auto nested_layers = DetectNestedVMLayers(layer);
            
            if (!nested_layers.empty()) {
                // 중첩 VM 발견
                for (auto nested : nested_layers) {
                    nested->parent_layer = layer;
                    layer->child_layers.push_back(nested);
                    vm_stack.push(nested);
                }
            } else {
                // VM 탈출 지점 탐지
                if (auto exit_point = DetectVMExit(layer)) {
                    if (layer->parent_layer == nullptr) {
                        // 최상위 레이어의 탈출 = OEP
                        MarkAsOEP(exit_point);
                        break;
                    } else {
                        // 부모 레이어로 복귀
                        vm_stack.pop();
                        current_layer = vm_stack.empty() ? nullptr : vm_stack.top();
                    }
                }
            }
        }
    }
    
private:
    std::vector<VMLayer*> DetectNestedVMLayers(VMLayer* parent) {
        std::vector<VMLayer*> nested_vms;
        
        // 핸들러 코드 내에서 또 다른 VM 진입 패턴 탐지
        for (auto handler_addr : parent->handlers) {
            auto handler_code = Disassemble(handler_addr);
            
            // VM 진입 시그니처 탐색
            if (ContainsVMEntrySignature(handler_code)) {
                auto nested_vm = new VMLayer();
                nested_vm->layer_id = parent->layer_id + 1;
                nested_vm->vm_entry = ExtractVMEntry(handler_code);
                nested_vms.push_back(nested_vm);
            }
        }
        
        return nested_vms;
    }
};
```

### 16. 실전 Themida 3.x 언패킹 전략

#### 16.1 Themida 특화 분석
```cpp
class ThemidaUnpacker {
private:
    // Themida의 알려진 특징
    struct ThemidaCharacteristics {
        bool uses_vm_protection;
        bool uses_mutation_engine;
        bool uses_anti_debugging;
        bool uses_code_encryption;
        bool uses_import_elimination;
        std::string vm_version;
    };
    
public:
    void UnpackThemida() {
        // 1단계: Themida 버전 식별
        auto characteristics = IdentifyThemidaVersion();
        
        // 2단계: VM 진입점 찾기
        auto vm_entry = LocateThemidaVMEntry();
        
        // 3단계: VM 핸들러 테이블 복원
        auto handler_table = ReconstructHandlerTable(vm_entry);
        
        // 4단계: 암호화된 코드 섹션 복호화
        DecryptCodeSections();
        
        // 5단계: IAT 재구성
        ReconstructImportTable();
        
        // 6단계: OEP 탐지
        auto oep = FindOriginalEntryPoint();
        
        // 7단계: 언패킹된 실행 파일 덤프
        DumpUnpackedExecutable(oep);
    }
    
private:
    DWORD_PTR LocateThemidaVMEntry() {
        // Themida VM 진입 시그니처
        std::vector<BytePattern> vm_entry_patterns = {
            // Themida 3.x 패턴
            {0x60, 0x9C, 0xE8, WILDCARD, WILDCARD, WILDCARD, WILDCARD},
            // 변형 패턴들...
        };
        
        for (auto& pattern : vm_entry_patterns) {
            auto matches = ScanMemoryForPattern(pattern);
            for (auto match : matches) {
                if (VerifyThemidaVMEntry(match)) {
                    return match;
                }
            }
        }
        
        return 0;
    }
    
    void DecryptCodeSections() {
        // Themida는 코드를 여러 섹션으로 나누어 암호화
        auto encrypted_sections = FindEncryptedSections();
        
        for (auto& section : encrypted_sections) {
            // 복호화 루틴 추적
            auto decryptor = FindDecryptorForSection(section);
            
            // 복호화 키 추출
            auto key = ExtractDecryptionKey(decryptor);
            
            // 섹션 복호화
            DecryptSection(section, key);
        }
    }
    
    void ReconstructImportTable() {
        // Themida는 IAT를 제거하고 직접 API 주소를 찾음
        std::map<std::string, DWORD_PTR> original_imports;
        
        // API 호출 추적
        TraceAPICalls([&](const APICall& call) {
            original_imports[call.api_name] = call.api_address;
        });
        
        // IAT 재구성
        BuildImportTable(original_imports);
    }
};
```

### 17. 자동화 도구 통합

#### 17.1 x64dbg 플러그인 아키텍처
```cpp
class VMUnpackerPlugin {
private:
    HybridOEPDetector oep_detector;
    ThemidaUnpacker themida_unpacker;
    PolymorphicHandlerAnalyzer handler_analyzer;
    EncryptedBytecodeAnalyzer bytecode_analyzer;
    
public:
    void InitializePlugin() {
        // x64dbg 콜백 등록
        RegisterDebugCallback(CB_BREAKPOINT, OnBreakpoint);
        RegisterDebugCallback(CB_STEPPED, OnSingleStep);
        RegisterDebugCallback(CB_EXCEPTION, OnException);
        RegisterDebugCallback(CB_MENUENTRY, OnMenuEntry);
    }
    
    void OnMenuEntry(int menu_id) {
        switch (menu_id) {
        case MENU_AUTO_UNPACK:
            AutoUnpackVM();
            break;
        case MENU_ANALYZE_HANDLERS:
            handler_analyzer.AnalyzePolymorphicHandlers();
            break;
        case MENU_DUMP_BYTECODE:
            bytecode_analyzer.DumpDecryptedBytecode();
            break;
        case MENU_FIND_OEP:
            auto oep = oep_detector.DetectOEP();
            DbgCmdExec(std::format("bp {}", oep).c_str());
            break;
        }
    }
    
private:
    void AutoUnpackVM() {
        // 자동 언패킹 프로세스
        ShowProgress("VM 보호 분석 중...");
        
        // 1. VM 타입 식별
        auto vm_type = IdentifyVMType();
        
        ShowProgress("핸들러 테이블 분석 중...");
        // 2. 핸들러 분석
        handler_analyzer.AnalyzePolymorphicHandlers();
        
        ShowProgress("바이트코드 복호화 중...");
        // 3. 바이트코드 처리
        bytecode_analyzer.AnalyzeEncryptedBytecode();
        
        ShowProgress("OEP 탐지 중...");
        // 4. OEP 탐지
        auto oep = oep_detector.DetectOEP();
        
        ShowProgress("언패킹 완료!");
        DbgCmdExec(std::format("dump {} {}", oep, "unpacked.exe").c_str());
    }
};
```

### 18. 성능 최적화 및 실전 팁

#### 18.1 선택적 추적 (Selective Tracing)
```cpp
class SelectiveTracer {
private:
    std::set<DWORD_PTR> trace_regions;  // 추적할 메모리 영역
    DWORD64 trace_counter = 0;
    DWORD64 max_trace_count = 1000000;  // 최대 추적 횟수
    
public:
    void OptimizedTrace() {
        // 전체 추적 대신 중요한 지점만 추적
        
        // 1. API 호출 지점
        SetAPIBreakpoints();
        
        // 2. 메모리 할당/해제
        HookMemoryOperations();
        
        // 3. 새 코드 영역 진입
        MonitorNewCodeRegions();
        
        // 4. VM 핸들러 전환 (샘플링)
        SampleHandlerTransitions(0.1);  // 10%만 추적
    }
    
    void SampleHandlerTransitions(double sample_rate) {
        // 모든 핸들러 전환을 추적하지 않고 샘플링
        static std::random_device rd;
        static std::mt19937 gen(rd());
        static std::uniform_real_distribution<> dis(0.0, 1.0);
        
        if (dis(gen) < sample_rate) {
            RecordHandlerTransition();
        }
    }
};
```

### 19. 종합 분석 체크리스트

언패킹 프로젝트를 수행할 때 다음 체크리스트를 따르세요:

#### 19.1 초기 분석
- [ ] 패커 타입 식별 (VMProtect, Themida, Code Virtualizer 등)
- [ ] 안티 디버깅 기법 파악
- [ ] VM 진입점 위치 확인
- [ ] 핸들러 테이블 위치 추정

#### 19.2 VM 구조 분석
- [ ] 핸들러 개수 및 타입 파악
- [ ] VM 레지스터 구조 이해
- [ ] 바이트코드 인코딩 방식 분석
- [ ] 디스패처 로직 분석

#### 19.3 코드 복원
- [ ] 암호화된 섹션 복호화
- [ ] 다형성 코드 정규화
- [ ] 제어 흐름 재구성
- [ ] IAT 재구성

#### 19.4 OEP 탐지
- [ ] 다층 분석 적용
- [ ] 후보 지점들 점수화
- [ ] 검증 및 확인
- [ ] 최종 OEP 결정

#### 19.5 덤핑 및 검증
- [ ] 메모리 덤프 생성
- [ ] PE 헤더 복구
- [ ] IAT 재구성 검증
- [ ] 덤프된 파일 실행 테스트

### 20. 참고 자료 및 도구

#### 20.1 권장 도구
- **x64dbg**: 동적 분석 및 디버깅
- **IDA Pro**: 정적 분석 및 디컴파일
- **Scylla**: IAT 재구성
- **PE-bear**: PE 파일 분석
- **Detect It Easy**: 패커 식별

#### 20.2 추가 학습 자료
- VMProtect 분석 논문 및 기술 문서
- Themida 보호 메커니즘 연구
- 가상화 난독화 이론
- Anti-anti-debugging 기법
- Code deobfuscation 기법

이러한 고급 기법들을 습득하고 조합하면, Themida 급의 가상화 보호도 효과적으로 분석하고 언패킹할 수 있습니다.
