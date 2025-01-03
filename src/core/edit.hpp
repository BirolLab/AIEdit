#pragma once

#include <cstddef>
#include <string>
#include <vector>

namespace aiedit {

class Edit
{
  public:

    enum class Type
    {
        NO_EDIT = '-',
        SUBSTITUION = 'S',
        DELETION = 'D',
        INSERTION = 'I',
    };
    
    static constexpr char NO_BASE = '.';

    const size_t pos;
    const char before, after;

    static const Edit substitution(size_t pos, char before, char after);
    static const Edit deletion(size_t pos, char deleted);
    static const Edit insertion(size_t pos, char inserted);

    [[nodiscard]] Type get_type() const;

    Edit& operator=(Edit other);

  private:

    Edit(size_t pos, char before, char after);
};

std::string apply_edits(const std::string& seq, const std::vector<Edit>& edits);

}