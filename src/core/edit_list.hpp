#pragma once

#include <vector>

#include "edit.hpp"

namespace aiedit {

class EditList
{

  public:

    EditList();

    void add();

    void sort();

  private:

    std::vector<Edit> edits;
};

}