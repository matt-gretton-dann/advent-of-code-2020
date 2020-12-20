#include <algorithm>
#include <array>
#include <cassert>
#include <iostream>
#include <list>
#include <map>
#include <regex>
#include <set>
#include <string>
#include <tuple>

using Id = unsigned long;
using EdgeHash = unsigned;
using EdgeHashes = std::vector<EdgeHash>;

enum Edge {
  Top,
  Left,
  Bottom,
  Right,
  FlippedTop,
  FlippedLeft,
  FlippedBottom,
  FlippedRight
};

EdgeHash flip(EdgeHash in) {
  EdgeHash out = 0;
  for (unsigned i = 0; i < 10; ++i) {
    bool set = (in & (1 << i)) != 0;
    out |= set << (9 - i);
  }
  return out;
}

/* A picture.
 *
 * We store the sides only in all combinations - calling them hashes.
 */
template <unsigned Size = 10> struct Picture {
  Picture(std::string const &header)
      : id_(std::stoul(header.substr(5))), row_(0), hashes_(8, 0),
        in_use_(false) {}

  void add_row(std::string const &row) {
    assert(row_ < Size);
    assert(row.size() == Size);
    if (row_ == 0) {
      for (auto c : row) {
        hashes_[Edge::Top] <<= 1;
        hashes_[Edge::Top] |= (c == '#');
      }
    } else if (row_ == Size - 1) {
      for (auto c : row) {
        hashes_[Edge::FlippedBottom] <<= 1;
        hashes_[Edge::FlippedBottom] |= (c == '#');
      }
    }
    hashes_[Edge::Right] <<= 1;
    hashes_[Edge::Right] |= (row[Size - 1] == '#');

    hashes_[Edge::FlippedLeft] <<= 1;
    hashes_[Edge::FlippedLeft] |= (row[0] == '#');

    ++row_;
    if (row_ == Size) {
      hashes_[Edge::FlippedTop] = ::flip(hashes_[Edge::Top]);
      hashes_[Edge::FlippedRight] = ::flip(hashes_[Edge::Right]);
      hashes_[Edge::Bottom] = ::flip(hashes_[Edge::FlippedBottom]);
      hashes_[Edge::Left] = ::flip(hashes_[Edge::FlippedLeft]);
    }
  }

  Id id() const noexcept { return id_; }

  EdgeHash hash(Edge e) const { return hashes_[e]; }
  EdgeHash hash_pair(Edge e1, Edge e2) const {
    return (hash(e1) << Size) + hash(e2);
  }

  void rotate() {
    auto t = hashes_[0];
    for (unsigned i = 0; i < 3; ++i) {
      hashes_[i] = hashes_[i + 1];
    }
    hashes_[3] = t;
  }

  void flip() {
    hashes_[0] = ::flip(hashes_[0]);
    hashes_[2] = ::flip(hashes_[2]);
    auto t = hashes_[1];
    hashes_[1] = ::flip(hashes_[3]);
    hashes_[3] = ::flip(t);
  }

  EdgeHashes::const_iterator begin() const { return hashes_.begin(); }
  EdgeHashes::const_iterator end() const { return hashes_.end(); }

  bool in_use() const { return in_use_; }
  void use() { in_use_ = true; }
  void release() { in_use_ = false; }

private:
  Id id_;
  std::size_t row_;
  EdgeHashes hashes_;
  bool in_use_;
};

template <unsigned PictureSize = 10> struct PictureArray {
  void add(Picture<PictureSize> const &pict) {
    auto id = pict.id();
    auto [it, success] = pictures_.insert({id, pict});
    assert(success);
    std::cout << "ID = " << id << " hashes:";
    for (auto hash : pict) {
      edge_map_.insert({hash, id});
      std::cout << " " << hash;
      auto [it, success] = hash_counts_.insert({hash, 1});
      if (!success) {
        it->second++;
      }
    }
    std::cout << "\n";
  }

  void print_unique_corners() const {
    std::map<Id, unsigned> id_counts;
    for (auto const &kv : hash_counts_) {
      if (kv.second == 1) {
        std::cout << "hash = " << kv.first << " count = " << kv.second
                  << " pictures =";
        auto [b, e] = edge_map_.equal_range(kv.first);
        while (b != e) {
          std::cout << " " << b->second;
          auto [it, success] = id_counts.insert({b->second, 1});
          if (!success) {
            it->second++;
          }
          ++b;
        }
        std::cout << "\n";
      }
    }

    for (auto const &kv : id_counts) {
      if (kv.second > 1) {
        std::cout << "Id = " << kv.first << " has " << kv.second
                  << " appearances\n";
      }
    }
  }

  unsigned width() const {
    if (pictures_.size() == 9) {
      return 3;
    } else if (pictures_.size() == 144) {
      return 12;
    } else {
      assert(false);
    }
  }

  void fill_array() {
    for (auto &picture : pictures_) {
      try_picture(0, 0, picture.second);
    }
  }

private:
  void try_picture(unsigned x, unsigned y, Picture<PictureSize> &pict) {
    if (pict.in_use()) {
      return;
    }
    pict.use();
    // Insert ourselves as an optimistic thing.
    array_.insert({{x, y}, pict.id()});

    for (unsigned flipped = 0; flipped < 2; ++flipped) {
      for (unsigned rotated = 0; rotated < 4; ++rotated) {
        pict.rotate();

        // Check that we fit on the left and top.
        if (x > 0) {
          auto left_id = array_.find({x - 1, y});
          assert(left_id != array_.end());
          auto left = pictures_.find(left_id->second);
          assert(left != pictures_.end());
          assert(left->first == left->second.id());
          if (left->second.hash(Edge::Right) != pict.hash(Edge::Left)) {
            continue;
          }
        }
        if (y > 0) {
          auto top_id = array_.find({x, y - 1});
          assert(top_id != array_.end());
          auto top = pictures_.find(top_id->second);
          assert(top != pictures_.end());
          if (top->second.hash(Edge::Bottom) != pict.hash(Edge::Top)) {
            continue;
          }
        }

        std::cout << "[" << x << ", " << y << "] = " << pict.id() << " ("
                  << pict.hash(Edge::Top) << ", " << pict.hash(Edge::Right)
                  << ", " << pict.hash(Edge::Bottom) << ", "
                  << pict.hash(Edge::Left) << ")\n";

        auto next_x = x + 1;
        auto next_y = y;
        if (next_x == width()) {
          ++next_y;
          next_x = 0;
        }
        if (next_y == width()) {
          auto top_left = array_.find({0, 0})->second;
          auto top_right = array_.find({0, width()})->second;
          auto bottom_left = array_.find({width(), 0})->second;
          auto bottom_right = array_.find({width(), width()})->second;
          std::cout << "SUCCESS: " << top_left << " * " << top_right << " * "
                    << bottom_left << " * " << bottom_right << " = "
                    << (top_left * top_right * bottom_left * bottom_right)
                    << "\n";
        } else {
          for (auto &p : pictures_) {
            try_picture(next_x, next_y, p.second);
          }
        }
      }
      pict.flip();
    }

    pict.release();
  }

  std::map<std::pair<unsigned, unsigned>, Id> array_;
  std::map<EdgeHash, unsigned> hash_counts_;
  std::multimap<EdgeHash, Id> edge_map_;
  std::map<Id, Picture<PictureSize>> pictures_;
};

int main(void) {
  std::string line;
  PictureArray array;

  while (std::getline(std::cin, line)) {
    if (line.empty()) {
      continue;
    }

    Picture pict(line);
    for (unsigned i = 0; i < 10; ++i) {
      std::getline(std::cin, line);
      pict.add_row(line);
    }

    array.add(pict);
  }

  array.print_unique_corners();
  array.fill_array();
  return 0;
}