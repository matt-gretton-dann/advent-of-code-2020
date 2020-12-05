#include <array>
#include <cassert>
#include <iostream>
#include <string>

/** \brief Return the position described by BSP in s.
 *  \tparam L2 log2 of the max position.
 *  \tparam LOWER character which means use lower half.
 *  \tparam UPPER character which means use upper half.
 *  \param  s     String to parse, must be L2 characters long
 *  \return       Position described by s.
 */
template <unsigned L2, char LOWER, char UPPER>
unsigned find_pos(std::string const &s) {
  assert(s.length() == L2);
  unsigned low = 0;
  unsigned high = 1 << L2;
  unsigned width = 1 << (L2 - 1);
  for (auto c : s) {
    assert(high != low);
    assert(width != 0);
    if (c == LOWER) {
      high = low + width;
    } else if (c == UPPER) {
      low = low + width;
    } else {
      assert(false);
    }
    width >>= 1;
  }

  assert(low == high - 1);
  return low;
}

template <unsigned L2R, unsigned L2C> struct Seat {
  Seat(std::string const &s) {
    assert(s.length() == L2R + L2C);
    row_ = find_pos<L2R, 'F', 'B'>(s.substr(0, L2R));
    col_ = find_pos<L2C, 'L', 'R'>(s.substr(L2R, L2C));
    std::cout << s << ": row " << row_ << ", column " << col_ << ", seat ID "
              << id() << ".\n";
  }

  unsigned id() const noexcept { return row_ * (1 << L2C) + col_; }

  unsigned row_;
  unsigned col_;
};

int main(int argc, char **argv) {
  constexpr unsigned l2r = 7;
  constexpr unsigned l2c = 3;
  constexpr unsigned max_id = 1 << (l2r + l2c);
  std::array<bool, max_id> id_present;
  for (unsigned i = 0; i < id_present.size(); ++i) {
    id_present[i] = false;
  }

  for (std::string line; std::getline(std::cin, line);) {
    Seat<l2r, l2c> seat(line);
    assert(seat.id() < max_id);
    id_present[seat.id()] = true;
  }

  for (unsigned id = 1; id < id_present.size() - 1; ++id) {
    if (!id_present[id] && id_present[id - 1] && id_present[id + 1]) {
      std::cout << "Empty seat at: " << id << "\n";
    }
  }

  return 0;
}