/*
 * Implementation of the mailbox.
 * Implementation notes: 
 *
 * The mailbox is protected with a lock to make sure that only 
 * one process is within the queue at any time. 
 *
 * It also uses condition variables to signal that more space or
 * more messages are available. 
 * In other words, this code can be seen as an example of implementing a
 * producer-consumer problem with a monitor and condition variables. 
 *
 * Note that this implementation only allows keys from 0 to 4 
 * (key >= 0 and key < MAX_Q). 
 *
 * The buffer is a circular array. 
*/

#include "common.h"
#include "thread.h"
#include "mbox.h"
#include "util.h"

mbox_t Q[MAX_MBOX];


/*
 * Returns the number of bytes available in the queue
 * Note: Mailboxes with count=0 messages should have head=tail, which
 * means that we return BUFFER_SIZE bytes.
 */
static int space_available(mbox_t * q)
{
  if ((q->tail == q->head) && (q->count != 0)) {
    /* Message in the queue, but no space  */
    return 0;
  }

  if (q->tail > q->head) {
    /* Head has wrapped around  */
    return q->tail - q->head;
  }
  /* Head has a higher index than tail  */
  return q->tail + BUFFER_SIZE - q->head;
}

/* 
*  Initialize mailbox system, called by kernel on startup
*  Initialize all the needed variables for each mailbox created
*/
void mbox_init(void)
{
  int i;
  for (i = 0; i < MAX_MBOX; i++)
  { 
    Q[i].used = 0;
    lock_init(&Q[i].l);
    Q[i].count = 0;
    Q[i].head = 0;
    Q[i].tail = 0;
    condition_init(&Q[i].moreSpace);
    condition_init(&Q[i].moreData);
    Q[i].buffer[BUFFER_SIZE];
    
  }
}

/*
 * Open a mailbox with the key 'key'. Returns a mailbox handle which
 * must be used to identify this mailbox in the following functions
 * (parameter q).
 */
int mbox_open(int key)
{
  // Check whether 'key' is a valid value
  if (key > MAX_MBOX - 1 || key < 0)
    return -1;
  else
  {
    lock_acquire(&Q[key].l);
    Q[key].used++;
    lock_release(&Q[key].l);
    return key;
  }
}

/* 
*  Close the mailbox with handle q
*  Decrease the 'used' value
*/
int mbox_close(int q)
{
  lock_acquire(&Q[q].l);
  Q[q].used--;
  lock_release(&Q[q].l);
  return 1;
}

/*
 * Get number of messages (count) and number of bytes available in the
 * mailbox buffer (space). Note that the buffer is also used for
 * storing the message headers, which means that a message will take
 * MSG_T_HEADER + m->size bytes in the buffer. (MSG_T_HEADER =
 * sizeof(msg_t header))
 */
int mbox_stat(int q, int *count, int *space)
{
  lock_acquire(&Q[q].l);
  *count = Q[q].count;
  *space = space_available(&Q[q]);
  lock_release(&Q[q].l);
  return 1;
}




/* 
*  Fetch a message from queue 'q' and store it in 'm'
*  To get the message from 'q', we first need to know how large the message is
*  Important to remember to wrap around the bounded buffer if needed
*/
int mbox_recv(int q, msg_t * m)
{

  lock_acquire(&Q[q].l);

  char *tmp = (char *)m;
  int i;
  // While there are no messages in the mailbox, wait
  while (Q[q].count == 0)
    condition_wait(&Q[q].l, &Q[q].moreData);

  // Get size of message
  for(i = 0; i < MSG_T_HEADER_SIZE; i++)
  {
    tmp[i] = Q[q].buffer[(Q[q].tail + i) % BUFFER_SIZE];
  }

  // Get the message
  for (i = 0; i < m->size; i++)
  {
    tmp[i+MSG_T_HEADER_SIZE] = Q[q].buffer[(Q[q].tail + i + MSG_T_HEADER_SIZE) % BUFFER_SIZE];
  }

  // Edit the tail to it's new value
  Q[q].tail = (Q[q].tail + MSG_SIZE(m)) % BUFFER_SIZE;
  Q[q].count--;
  // Let all the others know they can use this mailbox
  condition_broadcast(&Q[q].moreSpace);

  lock_release(&Q[q].l);

  return 1;
}

/* 
*  Insert 'm' into the mailbox 'q'
*  Make sure to not send while receiver's buffer is full
*  Send entire message, both header and the content itself
*/
int mbox_send(int q, msg_t * m)
{
  lock_acquire(&Q[q].l);

  char *tmp = (char *)m;
  int i;

  // While receiver is full, wait
  while(space_available(&Q[q]) < MSG_SIZE(m))
    condition_wait(&Q[q].l, &Q[q].moreSpace);

  // Send header
  for(i = 0; i < MSG_T_HEADER_SIZE; i++)
  {
    Q[q].buffer[(Q[q].head + i) % BUFFER_SIZE] = tmp[i];
  }

  // The content, important to wrap around buffer
  for(i = 0; i < m->size; i++)
  {
    Q[q].buffer[(Q[q].head + i + MSG_T_HEADER_SIZE) % BUFFER_SIZE] = m->body[i];
  }

  // Replace head with new head
  Q[q].head = (Q[q].head + MSG_SIZE(m)) % BUFFER_SIZE;
  Q[q].count++;
  condition_broadcast(&Q[q].moreData);
  lock_release(&Q[q].l);

  return 1;

}

