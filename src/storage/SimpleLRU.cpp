#include "SimpleLRU.h"
#include <iostream>

namespace Afina {
namespace Backend {
// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Put(const std::string &key, const std::string &value) {
  if(key.size() + value.size() > _max_size) {
    return false;
  }
  // std::cout << "Putting" << std::endl;

  cur_size += key.size() + value.size();
  while(cur_size > _max_size) { // creating space(LRU startegy) n^2? -fix?
    lru_node *lru_tail = _lru_head.get();
    while(lru_tail->next != nullptr) {
      lru_tail = lru_tail->next.get();
    }
    Delete(lru_tail->key);
  }

  if(_lru_head == nullptr) { // no head
    lru_node *node = new lru_node ({key, value, nullptr, nullptr});
    _lru_head = std::unique_ptr<lru_node>(node);
    _lru_index.emplace(std::reference_wrapper<const std::string> (_lru_head->key)
                  , std::reference_wrapper<lru_node>(*_lru_head));
    return true;
  }

  if(!Set(key, value)) { // sets deals with just new value
    lru_node *node = new lru_node ({key, value, nullptr, nullptr});
    // std::cout << "dgsdfg" << std::endl;
    _lru_head->prev = node; // dealing with standard case
    node->next = std::move(_lru_head);
    _lru_head = std::unique_ptr<lru_node>(node);
    _lru_index.emplace(std::reference_wrapper<const std::string>(_lru_head->key)
                  , std::reference_wrapper<lru_node>(*_lru_head));
    // if(node->next) {
    //   std::cout << " _lru_head->next->prev->key " << _lru_head->next->prev->key << std::endl;
    // }
    return true;
  }

  return true;
}


// See MapBasedGlobalLockImpl.h
bool SimpleLRU::PutIfAbsent(const std::string &key, const std::string &value) {
  auto it = _lru_index.find(std::reference_wrapper<const std::string>(key));
  if(it != _lru_index.end()) {
    return false;
  }
  Put(key, value);
  return true;
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Set(const std::string &key, const std::string &value) {
  // std::cout << "SETTTING " << key << " with " << value  << std::endl;
  auto it = _lru_index.find(std::reference_wrapper<const std::string>(key));
  if(it == _lru_index.end()) {
    // std::cout << "hello" << std::endl;
    return false;
  }
  // checck
  // std::cout << "value before:  " << _lru_head->value << std::endl;
  it->second.get().value = value;
  // updating LRU list
  Delete(it->second.get().key);
  Put(key, value);
  // lru_node &node = it->second.get();
  // std::cout << "Actual key: " << it->second.get().key << " and value " << it->second.get().value << std::endl;
  // std::cout << std::endl;
  // std::cout << std::endl;
  // if(it->second.get().prev != nullptr) {
  //   std::cout << "PRINTING BEFORE KEY" << it->second.get().prev->key << std::endl;
  // }
  // if(it->second.get().next != nullptr) {
  //   std::cout << "PRINTING AFTER KEY" << it->second.get().next->key << std::endl;
  // }
  // std::cout << std::endl;
  // std::cout << std::endl;
  //
  //
  // _lru_index.erase(it);
  // node.value = value;
  //
  //
  // std::unique_ptr<lru_node> node_add;
  // if(node.prev) {
  //   node_add = std::move(node.prev->next);
  //   if(node.next) {
  //     node.next->prev = node_add->prev;
  //     std::cout << "asdasdasdasdas.pred" << std::endl;
  //     node_add->prev = nullptr;
  //     std::cout << "IFnode.pred" << std::endl;
  //   }
  //   std::cout << "node_add key " << node_add->key << std::endl;
  //   node.prev->next = std::move(node.next); // change prev to next next
  // } else {
  //   std::cout << "value after:  " << _lru_head->value << std::endl;
  //   _lru_index.emplace(std::reference_wrapper<const std::string>(_lru_head->key)
  //                 , std::reference_wrapper<lru_node>(*_lru_head));
  //   return true;
  // }
  // // if(node.next) {
  // //   node.next->prev = node.prev; // change next to prev prev
  // // }
  //
  // // node.value = value;
  // // che-to v psevdo pute ne tak.
  // std::cout << "Node_add key: " << node_add->key << std::endl;
  // _lru_head->prev = node_add.get(); // dealing with standard case
  // node_add->next = std::move(_lru_head);
  // _lru_head = std::move(node_add);
  // std::cout << "Setting map with " << _lru_head->key << std::endl;
  // _lru_index.emplace(std::reference_wrapper<const std::string>(_lru_head->key)
  //               , std::reference_wrapper<lru_node>(*_lru_head));

  return true;
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Delete(const std::string &key) {
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
  } else {
    _lru_head = std::move(node.next);
  }
  return true;
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Get(const std::string &key, std::string &value) {
  auto it = _lru_index.find(std::reference_wrapper<const std::string>(key));
  if(it == _lru_index.end()) {
    return false;
  }

  value = it->second.get().value;

  //updating LRU list
  Delete(it->second.get().key);
  Put(key, value);
  return true;
}


} // namespace Backend
} // namespace Afina
