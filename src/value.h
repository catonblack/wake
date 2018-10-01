#ifndef VALUE_H
#define VALUE_H

#include "hash.h"
#include <string>
#include <memory>
#include <vector>
#include <cstdint>
#include <gmp.h>

/* Values */

struct Lambda;
struct Binding;
struct Location;
struct WorkQueue;

struct Value {
  const char *type;

  Value(const char *type_) : type(type_) { }
  virtual ~Value();

  std::string to_str() const; // one-line version
  virtual void format(std::ostream &os, int depth) const = 0; // depth=-1 means use one-line
  virtual Hash hash() const = 0;
};

std::ostream & operator << (std::ostream &os, const Value *value);

struct String : public Value {
  std::string value;

  static const char *type;
  String(const std::string &value_) : Value(type), value(value_) { }
  String(std::string &&value_) : Value(type), value(std::move(value_)) { }

  void format(std::ostream &os, int depth) const;
  Hash hash() const;
};

struct Integer : public Value {
  mpz_t value;

  static const char *type;
  Integer(const char *value_) : Value(type) { mpz_init_set_str(value, value_, 0); }
  Integer(long value_) : Value(type) { mpz_init_set_si(value, value_); }
  Integer() : Value(type) { mpz_init(value); }
  ~Integer();

  std::string str(int base = 10) const;
  void format(std::ostream &os, int depth) const;
  Hash hash() const;
};

struct Closure : public Value {
  Lambda *lambda;
  std::shared_ptr<Binding> binding;

  static const char *type;
  Closure(Lambda *lambda_, const std::shared_ptr<Binding> &binding_) : Value(type), lambda(lambda_), binding(binding_) { }
  void format(std::ostream &os, int depth) const;
  Hash hash() const;
};

struct Cause {
  std::string reason;
  std::vector<Location> stack;
  Cause(const std::string &reason_, std::vector<Location> &&stack_);
};

struct Exception : public Value {
  std::vector<std::shared_ptr<Cause> > causes;

  static const char *type;
  Exception() : Value(type) { }
  Exception(const std::string &reason, const std::shared_ptr<Binding> &binding);

  Exception &operator += (const Exception &other) {
    causes.insert(causes.end(), other.causes.begin(), other.causes.end());
    return *this;
  }

  void format(std::ostream &os, int depth) const;
  Hash hash() const;
};

#endif
