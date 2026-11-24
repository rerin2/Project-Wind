// 이 파일은 무엇을 하나요?
// 원자적 파일 쓰기의 실제 순서예요: 임시 파일 쓰기 → 닫고 확인 → 이름 바꿔 덮어쓰기.

#include "wind/save/atomic_file.hpp"

#include <fstream>
#include <system_error>

namespace wind::save {

std::filesystem::path temp_path_for(const std::filesystem::path& target) {
    auto temp = target;
    temp += ".tmp";
    return temp;
}

bool write_file_atomically(const std::filesystem::path& target, std::string_view bytes, std::string& error) {
    const auto temp = temp_path_for(target);
    // 1) 임시 파일에 다 써요 (이전에 남은 .tmp는 덮어써요)
    {
        std::ofstream out(temp, std::ios::binary | std::ios::trunc);
        if (!out) {
            error = "cannot open " + temp.string() + " for writing";
            return false;
        }
        out.write(bytes.data(), static_cast<std::streamsize>(bytes.size()));
        out.close();  // 닫아야 운영체제에 다 넘어가요. 닫은 뒤의 상태로 성공을 판단해요
        if (!out) {
            error = "failed to write " + temp.string();
            std::error_code ignored;
            std::filesystem::remove(temp, ignored);  // 반쯤 쓴 임시 파일은 치워요 (실패해도 본 파일엔 영향 없음)
            return false;
        }
    }
    // 2) 이름을 바꿔 본 파일을 한 번에 덮어써요 (MSVC: MoveFileExW + MOVEFILE_REPLACE_EXISTING)
    std::error_code ec;
    std::filesystem::rename(temp, target, ec);
    if (ec) {
        error = "cannot replace " + target.string() + ": " + ec.message();
        return false;
    }
    return true;
}

bool copy_file_atomically(const std::filesystem::path& from, const std::filesystem::path& target, std::string& error) {
    const auto temp = temp_path_for(target);
    std::error_code ec;
    // 1) 임시 파일로 복사 (남아 있던 .tmp는 덮어써요)
    std::filesystem::copy_file(from, temp, std::filesystem::copy_options::overwrite_existing, ec);
    if (ec) {
        error = "cannot copy " + from.string() + ": " + ec.message();
        return false;
    }
    // 2) 이름 바꿔 한 번에 덮어쓰기
    std::filesystem::rename(temp, target, ec);
    if (ec) {
        error = "cannot replace " + target.string() + ": " + ec.message();
        return false;
    }
    return true;
}

}  // namespace wind::save
