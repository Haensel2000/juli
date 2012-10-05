//import linked_list;

C int printf(char[] s, ...);

int main(char[][] args) 
{
  LinkedList l = createLinkedList();
  add(l, 4);
  add(l, 7);
  add(l, 2);
  print(l);
  remove(l, 5);
  print(l);
  remove(l, 2);
  print(l);
  remove(l, 7);
  print(l);
  remove(l, 3);
  print(l);
  remove(l, 4);
  print(l);
  add(l, 1);
  add(l, 2);
  print(l);
  return 0;
}
