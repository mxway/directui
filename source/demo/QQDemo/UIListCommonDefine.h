#ifndef DIRECTUI_UILISTCOMMONDEFINE_H
#define DIRECTUI_UILISTCOMMONDEFINE_H
#include <UIString.h>
#include <UIList.h>
#include <vector>

struct NodeData
{
    int level_;
    bool folder_;
    bool child_visible_;
    bool has_child_;
    UIString text_;
    UIString value;
    UIListContainerElement* list_elment_;
};

double CalculateDelay(double state);

class Node
{
public:
    Node();
    explicit Node(NodeData t);
    Node(NodeData t, Node* parent);
    ~Node();
    NodeData& data();
    int num_children() const;
    Node* child(int i);
    Node* parent();
    bool folder() const;
    bool has_children() const;
    void add_child(Node* child);
    void remove_child(Node* child);
    Node* get_last_child();

private:
    void set_parent(Node* parent);

private:
    typedef std::vector <Node*>	Children;

    Children	children_;
    Node*		parent_;

    NodeData    data_;
};

#endif //DIRECTUI_UILISTCOMMONDEFINE_H
