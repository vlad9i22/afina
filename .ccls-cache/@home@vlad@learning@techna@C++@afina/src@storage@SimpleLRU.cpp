#include "SimpleLRU.h"
#include <iostream>

namespace Afina {
namespace Backend {
// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Put(const std::string &key, const std::string &value) {
  std::cout << "Puting" << std::endl;
  lru_node *node = new lru_node ({key, value, nullptr, nullptr});
  if(key.size() + value.size() > _max_size) {
    return false;
  }

  cur_size += key.size() + value.size();
  // std::cout << cur_size << " " << _max_size << std::endl;
  while(cur_size > _max_size) { // creating space(LRU startegy) n^2? -fix?
    std::cout << "Deleteing to empty" << std::endl;
    lru_node *lru_tail = _lru_head.get();     // lru_node *lru_tail = &(*_lru_head);
    while(lru_tail->next != nullptr) {
      lru_tail = &(*lru_tail->next);
    }
    std::cout << lru_tail->value << "value" << std::endl;
    Delete(lru_tail->key);
    std::cout << "Delted to empty" << std::endl;
  }

  if(_lru_head == nullptr) { // no head
    // std::cout << "No head" << std::endl;
    _lru_head = std::unique_ptr<lru_node>(node);
    _lru_index.emplace(std::reference_wrapper<const std::string> (_lru_head->key)
                  , std::reference_wrapper<lru_node>(*_lru_head));
    return true;
  }

  if(!Set(key, value)) { // sets deals with just new value
    // std::cout << "shit" << std::endl;
    _lru_head->prev = node; // dealing with shit
    node->next = std::move(_lru_head);
    _lru_head = std::unique_ptr<lru_node>(node);
    _lru_index.emplace(std::reference_wrapper<const std::string> (_lru_head->key)
                  , std::reference_wrapper<lru_node>(*_lru_head));
    return true;
  }

  return true;
}


// See MapBasedGlobalLockImpl.h
bool SimpleLRU::PutIfAbsent(const std::string &key, const std::string &value) {
  std::cout << "Put" << std::endl;
  auto it = _lru_index.find(std::reference_wrapper<const std::string>(key));
  if(it != _lru_index.end()) {
    // std::cout << "OK" << std::endl;
    std::cout << "Put" << std::endl;
    return false;
  }
  // std::cout << "Setting" << std::endl;
  Put(key, value);
  std::cout << "Put" << std::endl;
  return true;
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Set(const std::string &key, const std::string &value) {
  std::cout << "Setting" << std::endl;
  auto it = _lru_index.find(std::reference_wrapper<const std::string>(key));
  if(it == _lru_index.end()) {
    std::cout << "Setted" << std::endl;
    return false;
  }
  cur_size += value.size() - it->second.get().value.size();
  it->second.get().value = value;
  std::cout << "Setted" << std::endl;
  return true;
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Delete(const std::string &key) {
  std::cout << "Deleting" << std::endl;
  auto it = _lru_index.find(std::reference_wrapper<const std::string>(key));

  if(it == _lru_index.end()) {
    return false;
  }

  cur_size -= it->second.get().key.size() + it->second.get().value.size();

  lru_node &node = it->second.get();
  _lru_index.erase(it);
  if(node.next) {
    node.next->prev = node.prev; // change next to prev prev
  }
  if(node.prev) {
    node.prev->next = std::move(node.next); // change prev to next next
  }

  // if(it->second.get().next) {
  //   it->second.get().next->prev = it->second.get().prev; // change next to prev prev
  // }
  // if(it->second.get().prev) {
  //   it->second.get().prev->next = std::move(it->second.get().next); // change prev to next next
  // }
  //
  // _lru_index.erase(it); // bad need to fix
  std::cout << "Deleted" << std::endl;
  return true;
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Get(const std::string &key, std::string &value) {
  std::cout << "Geting" << std::endl;
  auto it = _lru_index.find(std::reference_wrapper<const std::string>(key));
  if(it == _lru_index.end()) {
    std::cout << "Got" << std::endl;
    return false;
  }

  value = it->second.get().value;
  std::cout << "Got" << _max_size << std::endl;

  // std::cout << "value: " << value << std::endl;
  return true;
}

} // namespace Backend
} // namespace Afina
