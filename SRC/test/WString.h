#ifndef WSTRING_MOCK_H
#define WSTRING_MOCK_H

#include <string>

class WString {
 public:
  WString();
  WString(const char* cstr);
  WString(const std::string& str);
  WString(const WString& other);
  ~WString();

  const char* c_str() const;
  int length() const;
  bool equals(const char* cstr) const;
  bool equals(const std::string& str) const;
  bool equals(const WString& other) const;
  void clear();
  void reserve(int size);
  void concat(const char* cstr);
  void concat(const std::string& str);
  void concat(const WString& other);
  void operator+=(const char* cstr);
  void operator+=(const std::string& str);
  void operator+=(const WString& other);
  char operator[](int index) const;
  char& operator[](int index);

 private:
  std::string data;
};

#endif  // WSTRING_MOCK_H