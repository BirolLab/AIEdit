#include "edit_region_finder.hpp"

namespace aiedit {

EditRegionFinder::EditRegionFinder(const std::string_view seq,
                                   const std::shared_ptr<KmerModel>& kmer_model,
                                   float hit_threshold)
  : hash_fn(seq.data(), seq.size(), kmer_model->get_num_hashes(), kmer_model->get_kmer_size())
  , kmer_model(kmer_model)
  , hit_threshold(hit_threshold)
{
    next(true);
}

std::optional<std::pair<size_t, size_t>> EditRegionFinder::get_next_region()
{
    if (!next(false)) {
        return {};
    }
    size_t start_pos = hash_fn.get_pos();
    if (!next(true)) {
        return {};
    }
    size_t end_pos = hash_fn.get_pos();
    return std::make_pair(start_pos, end_pos);
}

bool EditRegionFinder::next(bool hit)
{
    while (hash_fn.roll()) {
        if ((kmer_model->score(hash_fn.hashes()) >= hit_threshold) == hit) {
            return true;
        }
    }
    return false;
}

EditRegionFinder::Iterator EditRegionFinder::begin()
{
    return EditRegionFinder::Iterator(*this, false);
}

EditRegionFinder::Iterator EditRegionFinder::end()
{
    return EditRegionFinder::Iterator(*this, true);
}

EditRegionFinder::Iterator::Iterator(EditRegionFinder& erf, bool done)
  : erf(erf)
{
    if (done) {
        current = std::make_pair<size_t, size_t>(0, 0);
    } else {
        current = erf.get_next_region().value_or(std::make_pair<size_t, size_t>(0, 0));
    }
}

std::pair<size_t, size_t> EditRegionFinder::Iterator::operator*() const { return current; }

EditRegionFinder::Iterator& EditRegionFinder::Iterator::operator++()
{
    current = erf.get_next_region().value_or(std::make_pair<size_t, size_t>(0, 0));
    return *this;
}

bool EditRegionFinder::Iterator::operator==(const EditRegionFinder::Iterator& other) const
{
    return current == other.current;
}

bool EditRegionFinder::Iterator::operator!=(const EditRegionFinder::Iterator& other) const
{
    return current != other.current;
}

}
