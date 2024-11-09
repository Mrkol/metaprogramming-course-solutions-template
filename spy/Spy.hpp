#pragma once

#include <concepts>
#include <memory>
#include <type_traits>
#include <cassert>

struct LoggerBase
{
  virtual LoggerBase* cp() = 0;
  virtual void Expire(size_t*) = 0;
  virtual ~LoggerBase() = default;
};

template <class T>
struct LoggerProxy: public LoggerBase
{
  // Both part
  LoggerProxy(const T&& foo)
    : log_f_(std::move(foo))
  {}

  LoggerProxy<T>* cp() override
  { return new LoggerProxy<T>(log_f_); }

  void Expire(size_t *cnt_) override
  {
    log_f_(*cnt_);
    *cnt_ = 0;
  }

  LoggerProxy& operator = (LoggerProxy&& bw)
  {
    LoggerProxy<T>&& ref = (LoggerProxy<T>&&)(bw);
    log_f_ = std::move(ref.log_f_);
    return *this;
  }

  template<typename U>
  LoggerProxy(U&& u)
    : log_f_(std::move(u))
  {}
  
  // Copyable extension
  LoggerProxy& operator = (const LoggerProxy& bw)
    requires std::copy_constructible<T>
  {
    log_f_ = ((const LoggerProxy<T>&)bw).log_f_;
    return *this;
  }

  LoggerProxy(const T& foo)
    requires std::copy_constructible<T>
    : log_f_(foo)
  {}

  T log_f_{};
};

// Allocator is only used in bonus SBO tests,
// ignore if you don't need bonus points.
template <class T /*, class Allocator = std::allocator<std::byte>*/ >
class Spy {
  using Logger = std::unique_ptr<LoggerBase>;

public:
  // if needed (see task readme):
  //   default constructor
  //   copy and move construction
  //   copy and move assignment
  //   equality operators
  //   destructor

  explicit Spy(T&& val /* , const Allocator& alloc = Allocator()*/ )
    : value_(std::move(val))
  {}

  Spy()
    // needed because Spy<Copyable> should be NOT
    //   std::semiregular<...>
    // requires std::is_default_constructible_v<T> && (std::move_constructible<T>)
    requires (std::is_default_constructible_v<T> && (std::move_constructible<T>))
    : value_()
    , logger_(nullptr)
  {};

  Spy(Spy&& s)
  : 
    value_(std::move(s.value_)),
    cnt_(s.cnt_),
    logger_(std::move(s.logger_))
  { s.cnt_ = 0; }

  Spy(const Spy& s)
    : value_(s.value_),
      cnt_(0)
  {
    // When we create a new s we check if it has logger
    // when it does we 
    logger_.reset(s.logger_->cp());
  }

  ~Spy()
  {
    if (cnt_) {
      logger_->Expire(&cnt_);
    }
  }

  void swap(Spy& f, Spy& s)
  { f.value_.swap(s.value_); }

  Spy& operator = (const Spy& s)
    requires std::copyable<T>
  {
    if(&s == this) {
      return *this;
    }
    
    value_ = s.value_;
    logger_.reset(nullptr);

    if(s.logger_.get() != nullptr){
      logger_.reset(s.logger_->cp());
    }
    
    cnt_ = 0;
    return *this;
  }

  Spy& operator = (Spy&& s)
    requires std::movable<T>
  {
    if (&s != this) {
      cnt_ = s.cnt_;
      value_ = std::move(s.value_);
      logger_ = std::move(s.logger_);
    }
    
    return *this;
  }
  
  bool operator == (const Spy<T>& s) const
    // to make class arbitrary constructible
    //   but not std::regular<Spy<semiregular>>
    //   we deprive method equality operator
    //   when <T> is not regular.
    requires std::regular<T>
  { return s.value_ == value_; }

  T& operator *()
  { return value_; }

  const T& operator *() const
  { return value_; }

  friend class SpyHelper;
  struct SpyHelper
  {
    SpyHelper(Spy<T>* spy_ptr)
      : spy_(spy_ptr)
    {}

    T* operator -> ()
    {
      if ((spy_->logger_).get() != nullptr){
        ++(spy_->cnt_);
      }
      return &(spy_->value_);
    }

    ~SpyHelper()
    {
      if (spy_->cnt_) {
        spy_->logger_->Expire(&spy_->cnt_);
      }
    }

    Spy<T>* spy_;
  };

  SpyHelper operator ->()
  {
    return SpyHelper(this);
  }

  template <class OtherLogger>
    requires (
      std::invocable<OtherLogger, unsigned int> &&
      std::move_constructible<OtherLogger> &&
      (!std::copy_constructible<T> || std::copy_constructible<OtherLogger>)
    )
  void setLogger(OtherLogger&& l)
  {
    logger_.reset(new LoggerProxy<OtherLogger>(std::forward<decltype(l)>(l)));
  }

private:
  // we should store not in logger because sometimes
  // we should know counter without logger
  T value_;
  size_t cnt_ = 0;
  Logger logger_{nullptr};
};
