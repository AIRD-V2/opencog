#include <ComboReduct/combo/vertex.h>
#include <iostream>
#include <sstream>

#include "ComboShellServer.h"
#include <boost/lexical_cast.hpp>
#include "ComboInterpreter.h"
#include "ComboProcedureRepository.h"
#include "StringMessage.h"
#include "PetComboVocabulary.h"

using namespace Procedure;
using namespace PetCombo;
using namespace boost;
using namespace std;
using namespace MessagingSystem;

ComboShellServer::ComboShellServer(const Control::SystemParameters &params)
  : NetworkElement(params,
		   params.get("COMBO_SHELL_ID"), 
		   params.get("COMBO_SHELL_IP"), 
		   lexical_cast<int>(params.get("COMBO_SHELL_PORT"))),
    _waiting(false)
{ }

bool ComboShellServer::processNextMessage(MessagingSystem::Message *msg){
  if (msg->getTo()!=getID())
    return false;
  string res=msg->getPlainTextRepresentation();
  
  if (res=="action_failure")
    cout << "execution failed!" << endl;
  else {
    if (res!="action_success")
      cout << "result: " << res << endl; 
    cout << "execution succeeded!" << endl;
  }
  _waiting=false;
  return false;
}

void ComboShellServer::idleTime() {
  if (_waiting) {
    if (haveUnreadMessage()) {
      MAIN_LOGGER.log(LADSUtil::Logger::DEBUG, "ComboShellServer - Requesting messages (idleTime()).");
      retrieveMessages(-1);
    } else {
      sleepUntilNextMessageArrives();
    }
    return;
  }
  
  combo_tree tr;
  if (!cin.good()) {
    cout << endl;
    exit(0);
  }

 start:
  cout << "> ";
  if (cin.peek()==' ' ||
      cin.peek()=='\n' ||
      cin.peek()=='\t')
    cin.get();
  while (cin.peek()==' ' ||
	 cin.peek()=='\n' ||
	 cin.peek()=='\t') {
    if (cin.peek()=='\n')
      cout << "> ";
    cin.get();
  }
  if (cin.peek()=='#') { //a comment line
    char tmp[1024];
    cin.getline(tmp,1024);
    goto start;
  }
  cin >> tr;
  if (!cin.good()) {
    cout << endl;
    exit(0);
  }

  try {
    stringstream ss;
    ss << tr;
    StringMessage msg(parameters.get("COMBO_SHELL_ID"),
		      "1", //to the OPC - this is a hack...
		      ss.str());
    cout << "sending schema " << ss.str() << "..." << endl;
    sendMessage(msg);
    cout << "schema sent to OPC, waiting for result ..." << endl;
    _waiting=true;
  } catch(...) {
    cout << "execution failed (threw exception)" << endl;
  }
}
