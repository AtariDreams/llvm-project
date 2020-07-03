struct One {
  int one = 142;
  constexpr One() = default;
  virtual ~One();
};

struct Two : One {
  int two = 242;
  constexpr Two() = default;
  ~Two() override;
};

namespace member {
struct One {
  int member = 147;
  constexpr One() = default;
  virtual ~One();
};

struct Two {
  One one;
  int member = 247;
  constexpr Two() = default;
  virtual ~Two();
};
} // namespace member
