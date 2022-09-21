struct MY_QUEUEABLE_STRUCT
{
    // QUEUE_Node must be the first struct member.
    struct QUEUE_Node node;
    // Other members.
    uint32_t a;
    uint32_t b;
    uint32_t c;
};

struct QUEUE queue;
QUEUE_Init (&queue);

struct MY_QUEUEABLE_STRUCT myqst1;
struct MY_QUEUEABLE_STRUCT myqst2;

// This node will be removed.
mysqt2.a = 1;

// Cast the MY_QUEUEABLE_STRUCT pointer to QUEUE_Node on insertion.
QUEUE_NodeEnqueue (&queue, (struct QUEUE_Node *)&myqst1);
QUEUE_NodeEnqueue (&queue, (struct QUEUE_Node *)&myqst2);

// Queue traverse.
struct QUEUE_TRV t;
QUEUE_TRV_Init (&t, &queue, QUEUE_TRV_Dir_BackToFront);

// Cast the QUEUE_Node pointer back to MY_QUEUEABLE_STRUCT on traversal.
struct QUEUE_Node *n;
while ((n = QUEUE_TRV_Step(&t)) != NULL)
{
    struct MY_QUEUEABLE_STRUCT *myn = (struct MY_QUEUEABLE_STRUCT *) n;
    // Member access and node detach.
    if (myn->a == 1)
    {
        QUEUE_NodeDetach (&queue, n);
    }
} 
