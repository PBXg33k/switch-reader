#include "Dialog.hpp"
#include "Ui.hpp"

bool Dialog_Confirm::get_bool(std::string message){
  printf("Getting response\n");
  int res = Dialog::get_response(message, {"No", "Yes"});
  printf("Response was %d\n", res);

  if(res == 1)
    return true;
  else
    return false;
}