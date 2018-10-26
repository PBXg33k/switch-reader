#include "Search.hpp"

int SearchBrowser::active_elem = 0;

void SearchBrowser::set_touch(){

}

Handler SearchBrowser::on_event(int val){
  if(val > 0 && val <= 26){
    // Keyboard input
  }

  if(val > 26){
    // Button input
  }
  return Handler::Search;
}

void SearchBrowser::render(){

}
