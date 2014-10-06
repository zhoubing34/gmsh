#include "onelab2Group.h"

#include "FlGui.h"
#include <FL/Fl_Box.H>
#include <FL/Fl_Check_Button.H>
#include <FL/Fl_Input_Choice.H>
#include <FL/Fl_Choice.H>
#include <FL/Fl_Menu_Item.H>
#include "inputRange.h"
#include "outputRange.h"
#include "inputRegion.h"

#include "OnelabDatabase.h"
#include "Options.h"
#include "Context.h"

OnelabDatabase *OnelabDatabase::_instance = NULL;

void connect_cb(Fl_Widget *w, void *arg)
{
  onelab2Group *obj = (onelab2Group *)arg;
  obj->clearTree();
  GmshNetworkClient *cli = OnelabDatabase::instance()->useAsNetworkClient(obj->getServerIP(), obj->getServerPort());
  cli->setCallback(obj);
  // FIXME just for debug
  FILE *fp = fopen("/tmp/onelab.db", "rb");
  if(fp){
    std::cout << "Get database from onelab.db" << std::endl;
    OnelabDatabase::instance()->fromFile(fp);
    fclose(fp);
  }
}

void onelab2_cb(Fl_Widget *w, void *data)
{
  if(!data) return;

  std::string action((const char*)data);
  OnelabDatabase::instance()->onelab_cb(action);
}

static bool getFlColor(const std::string &str, Fl_Color &c)
{
  if(str == "1"){
    c = FL_YELLOW;
    return true;
  }
  int r, g, b;
  if(str.size() && GetRGBForString(str.c_str(), r, g, b)){
    c = fl_color_cube(r * (FL_NUM_RED - 1) / 255,
                      g * (FL_NUM_GREEN - 1) / 255,
                      b * (FL_NUM_BLUE - 1) / 255);
    return true;
  }
  c = FL_BLACK;
  return false;
}

onelab2Group::onelab2Group(int x, int y, int w, int h, const char *l)
  : Fl_Group(x, y, w, h, l), _stop(false), _enableTreeWidgetResize(false)
{
  int col = FL_BACKGROUND2_COLOR;
  color(col);

  box(GMSH_SIMPLE_RIGHT_BOX);
  int dx = Fl::box_dx(box());
  int dy = Fl::box_dy(box());
  int dw = Fl::box_dw(box());
  int dh = Fl::box_dh(box());

  _tree = new Fl_Tree(x + dx, y + dy + 6*BH, w - dw, h - dh - BH - 2 * WB - 6*BH);
  _tree->color(col);
  // TODO _tree->callback(onelab_tree_cb);
  _tree->connectorstyle(FL_TREE_CONNECTOR_SOLID);
  _tree->showroot(0);
  _tree->box(FL_FLAT_BOX);
  _tree->scrollbar_size(std::max(10, FL_NORMAL_SIZE - 2));
  _tree->end();

  int BB2 = BB / 2 + 4;
  _butt[0] = new Fl_Button(x + w - 3 * WB - 3 * BB2, y + h - WB - BH, BB2, BH, "Check");
  _butt[0]->callback(onelab2_cb, (void*)"check");

  _butt[1] = new Fl_Button(x + w - 2 * WB - 2 * BB2, y + h - WB - BH, BB2, BH, "Run");
  _butt[1]->callback(onelab2_cb, (void*)"compute");

  Fl_Check_Button *useServer = new Fl_Check_Button(x+WB, y, w-2*WB, BH, "Use a distant server"); // TODO
  Fl_Box *ip_lbl = new Fl_Box(x+WB , y+BH, w-2*WB, BH, "Server IP address:");
  server_ip = new Fl_Input(x+WB, y+2*BH, w-2*WB, BH, "ip");
  server_ip->value("127.0.0.1");
  Fl_Box *port_lbl = new Fl_Box(x+WB , y+3*BH, w-2*WB, BH, "Server port:");
  server_port = new Fl_Input(x+WB, y+4*BH, w-2*WB, BH, "port");
  server_port->value("3456");
  Fl_Button *connect_btn = new Fl_Button(x+WB, y+5*BH, w-2*WB, BH, "Connect");
  connect_btn->callback(connect_cb, this);

  _computeWidths();
  _widgetLabelRatio = 0.48;
  OnelabDatabase::instance()->useAsClient()->setCallback(this);
}
onelab2Group::~onelab2Group()
{
  Fl::delete_widget(_tree);
}

void onelab2Group::clearTree(bool deleteWidgets)
{
  _tree->clear();
  _tree->sortorder(FL_TREE_SORT_ASCENDING);
  _tree->selectmode(FL_TREE_SELECT_NONE);

  std::vector<Fl_Widget*> delWidgets;
  std::vector<char*> delStrings;
  if(deleteWidgets){
    delWidgets = _treeWidgets;
    delStrings = _treeStrings;
    _treeWidgets.clear();
    _treeStrings.clear();
   }
  FlGui::check();
  if(deleteWidgets){
    for(unsigned int i = 0; i < delWidgets.size(); i++)
      Fl::delete_widget(delWidgets[i]);
    for(unsigned int i = 0; i < delStrings.size(); i++)
      free(delStrings[i]);
  }
}

template <class T>
void onelab2Group::addParameter(T &p)
{
  std::cout << "add " << p.getName() << " to the tree (visible = " << p.getVisible() << ')' << std::endl;
  if(!p.getVisible() || CTX::instance()->solver.showInvisibleParameters) return;
  bool highlight = false;
  Fl_Color c;
  if(getFlColor(p.getAttribute("Highlight"), c)) highlight = true;
  Fl_Tree_Item *n = _tree->add(p.getName().c_str());
  n->labelsize(FL_NORMAL_SIZE + 4);
  int ww = _baseWidth - (n->depth() + 1) * _indent;
  ww *= _widgetLabelRatio; // FIXME CHANGE THIS
  int hh = n->labelsize() + 4;
  Fl_Group *grp = new Fl_Group(1, 1, ww, hh);
  Fl_Widget *widget = _addParameterWidget(p, ww, hh, n, highlight, c);
  grp->end();
  if(!_enableTreeWidgetResize) grp->resizable(0);
  _treeWidgets.push_back(grp);
  widget->copy_label(p.getShortName().c_str());
  std::string help = p.getLabel().size() ? p.getLabel() : p.getShortName();
  if(p.getHelp().size()) help += ":\n" + p.getHelp();
  widget->copy_tooltip(help.c_str());
  n->widget(grp);
  _tree->end();
  _tree->redraw();
}
Fl_Widget *onelab2Group::_addParameterWidget(onelab::parameter &p, int ww, int hh, Fl_Tree_Item *n, bool highlight, Fl_Color c)
{
  int type = p.getAttributeType();
  if(type == onelab::number::attributeType())
      return _addParameterWidget(*(onelab::number *)&p, ww, hh, n, highlight, c);
  if(type == onelab::string::attributeType())
      return _addParameterWidget(*(onelab::string *)&p, ww, hh, n, highlight, c);
  if(type == onelab::region::attributeType())
      return _addParameterWidget(*(onelab::region *)&p, ww, hh, n, highlight, c);
  if(type == onelab::function::attributeType())
      return _addParameterWidget(*(onelab::function *)&p, ww, hh, n, highlight, c);
  return NULL;
}
// callback for number
static void onelab_number_input_range_cb(Fl_Widget *w, void *data)
{
  if(!data) return;
  std::string name((char*)data);
  std::cout << name << std::endl;
  std::vector<onelab::number> numbers;
  OnelabDatabase::instance()->get(numbers, name);
  if(numbers.size()){
    inputRange *o = (inputRange*)w;
    onelab::number old = numbers[0];
    if(o->doCallbackOnValues()){
      numbers[0].setValue(o->value());
      numbers[0].setMin(o->minimum());
      numbers[0].setMax(o->maximum());
      numbers[0].setStep(o->step());
      numbers[0].setChoices(o->choices());
    }
    o->doCallbackOnValues(true);
    numbers[0].setAttribute("Loop", o->loop());
    numbers[0].setAttribute("Graph", o->graph());
    //setGmshOption(numbers[0]);
    OnelabDatabase::instance()->set(numbers[0]);
    //updateGraphs();
    //autoCheck(old, numbers[0]);
  }
}
static void onelab_number_choice_cb(Fl_Widget *w, void *data)
{
  if(!data) return;
  std::string name((char*)data);
  std::vector<onelab::number> numbers;
  OnelabDatabase::instance()->get(numbers, name);
  if(numbers.size()){
    Fl_Choice *o = (Fl_Choice*)w;
    std::vector<double> choices = numbers[0].getChoices();
    onelab::number old = numbers[0];
    if(o->value() < (int)choices.size()) numbers[0].setValue(choices[o->value()]);
    //setGmshOption(numbers[0]);
    OnelabDatabase::instance()->set(numbers[0]);
    //autoCheck(old, numbers[0]);
  }
}
static void onelab_number_check_button_cb(Fl_Widget *w, void *data)
{
  if(!data) return;
  std::string name((char*)data);
  std::vector<onelab::number> numbers;
  OnelabDatabase::instance()->get(numbers, name);
  if(numbers.size()){
    Fl_Check_Button *o = (Fl_Check_Button*)w;
    onelab::number old = numbers[0];
    numbers[0].setValue(o->value());
    //setGmshOption(numbers[0]);
    OnelabDatabase::instance()->set(numbers[0]);
    //autoCheck(old, numbers[0]);
  }
}
// add a parameter number to the tree
Fl_Widget *onelab2Group::_addParameterWidget(onelab::number &p, int ww, int hh, Fl_Tree_Item *n, bool highlight, Fl_Color c)
{
  char *path = strdup(getPath(n).c_str());
  _treeStrings.push_back(path);

  // enumeration (display choices as value labels, not numbers)
  if(p.getChoices().size() &&
     p.getChoices().size() == p.getValueLabels().size()){
    Fl_Choice *but = new Fl_Choice(1, 1, ww, hh);
    std::vector<Fl_Menu_Item> menu;
    std::map<double, std::string> labels(p.getValueLabels());
    for(std::map<double, std::string>::iterator it = labels.begin();
        it != labels.end(); it++){
      char *str = strdup(it->second.c_str());
      _treeStrings.push_back(str);
      Fl_Menu_Item menuItem = {str, 0, 0, 0, 0};
      if(highlight) menuItem.labelcolor(c);
      menu.push_back(menuItem);
    }
    Fl_Menu_Item it = {0};
    menu.push_back(it);
    but->copy(&menu[0]);
    for(unsigned int i = 0; i < p.getChoices().size(); i++){
      if(p.getValue() == p.getChoices()[i]){
        but->value(i);
        break;
      }
    }
    but->callback(onelab_number_choice_cb, (void*)path);
    but->align(FL_ALIGN_RIGHT);
    if(p.getReadOnly()) but->deactivate();
    return but;
  }

  // check box (boolean choice)
  if(p.getChoices().size() == 2 &&
     p.getChoices()[0] == 0 && p.getChoices()[1] == 1){
    n->labelsize(FL_NORMAL_SIZE + 2);
    Fl_Check_Button *but = new Fl_Check_Button(1, 1, ww / _widgetLabelRatio, hh);
    but->box(FL_FLAT_BOX);
    but->color(_tree->color());
    but->value(p.getValue());
    but->callback(onelab_number_check_button_cb, (void*)path);
    if(highlight) but->color(c);
    if(p.getReadOnly()) but->deactivate();
    return but;
  }

  // non-editable value
  if(p.getReadOnly()){
    outputRange *but = new outputRange(1, 1, ww, hh);
    //TODO but->callback(onelab_number_output_range_cb, (void*)path);
    but->value(p.getValue());
    but->align(FL_ALIGN_RIGHT);
    but->graph(p.getAttribute("Graph"));
    if(highlight) but->color(c);
    return but;
  }

  // general number input
  inputRange *but = new inputRange(1, 1, ww, hh, onelab::parameter::maxNumber(),
                                   p.getAttribute("ReadOnlyRange") == "1");
  but->value(p.getValue());
  but->minimum(p.getMin());
  but->maximum(p.getMax());
  but->step(p.getStep());
  but->choices(p.getChoices());
  but->loop(p.getAttribute("Loop"));
  but->graph(p.getAttribute("Graph"));
  but->callback(onelab_number_input_range_cb, (void*)path);
  but->when(FL_WHEN_RELEASE | FL_WHEN_ENTER_KEY);
  but->align(FL_ALIGN_RIGHT);
  if(highlight) but->color(c);
  return but;
}
// callback for string
static void onelab_string_input_choice_cb(Fl_Widget *w, void *data)
{
  if(!data) return;
  std::string name((char*)data);
  std::vector<onelab::string> strings;
  OnelabDatabase::instance()->get(strings, name);
  if(strings.size()){
    Fl_Input_Choice *o = (Fl_Input_Choice*)w;
    onelab::string old = strings[0];
    strings[0].setValue(o->value());
    std::string choices;
    for(int i = 0; i < o->menubutton()->menu()->size(); i++){
      if(o->menubutton()->menu()[i].flags & FL_MENU_TOGGLE){
        if(o->menubutton()->menu()[i].flags & FL_MENU_VALUE)
          choices += "1";
        else
          choices += "0";
      }
    }
    if(choices.size())
      strings[0].setAttribute("MultipleSelection", choices);
    //setGmshOption(strings[0]);
    OnelabDatabase::instance()->set(strings[0]);
    //autoCheck(old, strings[0]);
  }
}
// add parameter string to tree
Fl_Widget *onelab2Group::_addParameterWidget(onelab::string &p, int ww, int hh,
                                            Fl_Tree_Item *n, bool highlight, Fl_Color c)
{
  char *path = strdup(getPath(n).c_str());
  _treeStrings.push_back(path);

  // macro button
  if(p.getAttribute("Macro") == "Gmsh"){
    Fl_Button *but = new Fl_Button(1, 1, ww / _widgetLabelRatio, hh);
    but->box(FL_FLAT_BOX);
    but->color(_tree->color());
    but->selection_color(_tree->color());
    but->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE | FL_ALIGN_CLIP);
    //TODO but->callback(onelab_string_button_cb, (void*)path);
    if(highlight) but->color(c);
    return but;
  }

  // non-editable value
  if(p.getReadOnly()){
    Fl_Output *but = new Fl_Output(1, 1, ww, hh);
    but->value(p.getValue().c_str());
    but->align(FL_ALIGN_RIGHT);
    if(highlight) but->color(c);
    return but;
  }

  // simple string (no menu)
  if(p.getChoices().empty() && p.getKind() != "file"){
    Fl_Input *but = new Fl_Input(1, 1, ww, hh);
    but->value(p.getValue().c_str());
    //TODO but->callback(onelab_string_input_cb, (void*)path);
    but->when(FL_WHEN_ENTER_KEY);
    but->align(FL_ALIGN_RIGHT);
    if(highlight) but->color(c);
    return but;
  }

  // general string input
  Fl_Input_Choice *but = new Fl_Input_Choice(1, 1, ww, hh);
  std::string multipleSelection = p.getAttribute("MultipleSelection");
  if(multipleSelection.size())
    ;//but->menubutton()->callback(multiple_selection_menu_cb, but);
  std::vector<Fl_Menu_Item> menu;
  for(unsigned int j = 0; j < p.getChoices().size(); j++){
    char *str = strdup(p.getChoices()[j].c_str());
    _treeStrings.push_back(str);
    bool divider = (p.getKind() == "file" &&
                    j == p.getChoices().size() - 1);
    int choice = multipleSelection.size() ? FL_MENU_TOGGLE : 0;
    if(multipleSelection.size() > j && multipleSelection[j] == '1')
      choice |= FL_MENU_VALUE;
    Fl_Menu_Item it = {str, 0, 0, 0, divider ? FL_MENU_DIVIDER : choice};
    menu.push_back(it);
  }
  //if(p.getKind() == "file"){
  //  Fl_Menu_Item it = {"Choose...", 0, onelab_input_choice_file_chooser_cb, (void*)n};
  //  menu.push_back(it);
  //  Fl_Menu_Item it2 = {"Edit...", 0, onelab_input_choice_file_edit_cb, (void*)n};
  //  menu.push_back(it2);
  //  if(GuessFileFormatFromFileName(p.getValue()) >= 0){
  //    Fl_Menu_Item it3 = {"Merge...", 0, onelab_input_choice_file_merge_cb, (void*)n};
  //    menu.push_back(it3);
  //  }
  //}
  Fl_Menu_Item it = {0};
  menu.push_back(it);
  but->menubutton()->copy(&menu[0]);
  but->value(p.getValue().c_str());
  but->callback(onelab_string_input_choice_cb, (void*)path);
  but->input()->when(FL_WHEN_ENTER_KEY);
  but->align(FL_ALIGN_RIGHT);
  if(highlight) but->input()->color(c);
  return but;
}
Fl_Widget *onelab2Group::_addParameterWidget(onelab::region &p, int ww, int hh,
                                            Fl_Tree_Item *n, bool highlight, Fl_Color c)
{
  char *path = strdup(getPath(n).c_str());
  _treeStrings.push_back(path);

  // non-editable value
  if(p.getReadOnly()){
    inputRegion *but = new inputRegion(1, 1, ww, hh, true);
    but->value(p.getValue());
    but->align(FL_ALIGN_RIGHT);
    if(highlight) but->color(c);
    return but;
  }

  inputRegion *but = new inputRegion(1, 1, ww, hh, false);
  but->value(p.getValue());
  but->align(FL_ALIGN_RIGHT);
  //TODO but->callback(onelab_region_input_cb, (void*)path);
  if(highlight) but->color(c);
  return but;
}

Fl_Widget *onelab2Group::_addParameterWidget(onelab::function &p, int ww, int hh,
                                            Fl_Tree_Item *n, bool highlight, Fl_Color c)
{
  // non-editable value
  if(1 || p.getReadOnly()){
    Fl_Output *but = new Fl_Output(1, 1, ww, hh);
    but->value("TODO function");
    but->align(FL_ALIGN_RIGHT);
    if(highlight) but->color(c);
    return but;
  }
}

void onelab2Group::updateParameter(onelab::parameter &p)
{
  int type = p.getAttributeType();
  if(type == onelab::number::attributeType())
      return updateParameter(*(onelab::number *)&p);
  if(type == onelab::string::attributeType())
      return updateParameter(*(onelab::string *)&p);
}
void onelab2Group::updateParameter(onelab::number &p)
{
  Fl_Tree_Item *n = _tree->find_item(p.getName().c_str());
  if(!n) {
    addParameter(p);
    return;
  }
  Fl_Group *grp = (Fl_Group *)n->widget();
  // enumeration (display choices as value labels, not numbers)
  if(p.getChoices().size() &&
     p.getChoices().size() == p.getValueLabels().size()){
    Fl_Choice *but = (Fl_Choice *)grp->child(0);
    //std::vector<Fl_Menu_Item> menu;
    //std::map<double, std::string> labels(p.getValueLabels());
    //for(std::map<double, std::string>::iterator it = labels.begin();
    //    it != labels.end(); it++){
    //  char *str = strdup(it->second.c_str());
    //  _treeStrings.push_back(str);
    //  Fl_Menu_Item menuItem = {str, 0, 0, 0, 0};
    //  if(highlight) menuItem.labelcolor(c);
    //  menu.push_back(menuItem);
    //}
    //Fl_Menu_Item it = {0};
    //menu.push_back(it);
    //but->copy(&menu[0]);
    for(unsigned int i = 0; i < p.getChoices().size(); i++){
      if(p.getValue() == p.getChoices()[i]){
        but->value(i);
        break;
      }
    }
    return;
  }

  // check box (boolean choice)
  if(p.getChoices().size() == 2 &&
     p.getChoices()[0] == 0 && p.getChoices()[1] == 1){
    Fl_Check_Button *but = (Fl_Check_Button *)grp->child(0);
    but->value(p.getValue());
    return;
  }

  // non-editable value FIXME
  if(p.getReadOnly()){
    outputRange *but = (outputRange *)grp->child(0);;
    but->value(p.getValue());
    but->graph(p.getAttribute("Graph"));
    return;
  }

  // general number input
  inputRange *but = (inputRange *)grp->child(0);
  but->value(p.getValue());
  but->minimum(p.getMin());
  but->maximum(p.getMax());
  but->step(p.getStep());
  but->choices(p.getChoices());
  but->loop(p.getAttribute("Loop"));
  but->graph(p.getAttribute("Graph"));
}
void onelab2Group::updateParameter(onelab::string &p)
{
  Fl_Tree_Item *n = _tree->find_item(p.getName().c_str());
  if(!n) {
    addParameter(p);
    return;
  }
  Fl_Group *grp = (Fl_Group *)n->widget();
  // macro button
  if(p.getAttribute("Macro") == "Gmsh"){
    return;
  }

  // non-editable value FIXME
  if(p.getReadOnly()){
    Fl_Output *but = (Fl_Output *)grp->child(0);
    but->value(p.getValue().c_str());
    return;
  }

  // simple string (no menu)
  if(p.getChoices().empty() && p.getKind() != "file"){
    Fl_Input *but = (Fl_Input *)grp->child(0);
    but->value(p.getValue().c_str());
    return;
  }

  // general string input TODO
  Fl_Input_Choice *but = (Fl_Input_Choice *)grp->child(0);
  but->value(p.getValue().c_str());
}

void onelab2Group::removeParameter(onelab::parameter &p)
{
  Fl_Tree_Item *n = _tree->find_item(p.getName().c_str());
  _tree->remove(n);
}

void onelab2Group::_computeWidths()
{
  _baseWidth = (int)(_tree->w() - _tree->marginleft());
  _indent = (int)(_tree->connectorwidth() / 2. + _tree->openicon()->w() / 2.);
}

std::string onelab2Group::getPath(Fl_Tree_Item *item)
{
  if(!item){
    Msg::Error("No item for path");
    return "";
  }
  char path[1024];
  if(_tree->item_pathname(path, sizeof(path), item)){
    Msg::Error("Could not get path for item");
    return "";
  }
  return std::string(path);
}

