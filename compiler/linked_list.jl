C int printf(char[] s, ...);

struct ListItem {
  ListItem next;
  int value;
}

struct LinkedList
{
  ListItem begin;
  ListItem end;
}

void print(LinkedList l)
{
  ListItem i = l.begin;
  if (l.begin != null)
  {
    printf("begin = %d; ", l.begin.value);
  }
  else
  {
    printf("begin = null; ");
  }
  if (l.end != null)
  {
    printf("end = %d;   ", l.end.value);
  }
  else
  {
    printf("end = null;   ");
  }
  while (i != null)
  {
    printf("%d ", i.value);
    i = i.next;
  }
  printf("\n");
  return;
}


void add(LinkedList l, int value)
{
  ListItem n = new ListItem;
  n.value = value;
  if (l.end != null)
  {
    l.end.next = n;
  } 
  else
  {
    l.begin = n;
  }
  l.end = n;
  return;
}

void remove(LinkedList l, int value)
{
  ListItem p = null;
  ListItem c = l.begin;
  while (c != null and c.value != value)
  {
    p = c;
    c = c.next;
  }
  
  if (c != null)
  {
    ListItem last;
    if (p == null)
    {
      l.begin = c.next;
    } 
    else
    {
      p.next = c.next;
    }
    
    if (c.next == null)
    {
      l.end = p;
    }
   
  }
  return;
}

LinkedList createLinkedList()
{
  LinkedList result = new LinkedList;
  result.begin = null;
  result.end = null;
  return result;
}

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
