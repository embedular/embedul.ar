// "s" is an instance of a hypothetical STREAM implementation to handle a
// particular hardware UART. Suppose this UART has a limited baud rate and
// TX buffering, so it may not send all cyclic elements at once. That is why
// CYCLIC_OUT_ToStream() may return before consuming every element.
//
// "retries" is the maximum number of times to keep calling
// CYCLIC_OUT_ToStream() after a STREAM EOF.

uint32_t retries = 10;

while (retries --)
{
    // Consume elements from the cyclic instance "c" and send them to the
    // STREAM "s".
    CYCLIC_OUT_ToStream (&c, &s);
    // Call returned.
    
    // Check for STREAM EOF
    if (!STREAM_EOF (&s))
    {
        // Not an EOF. CYCLIC_OUT_ToStream should have consumed and sent all
        // elements.
        // TEST: Assert that all elements have been consumed.
        BOARD_AssertState (CYCLIC_Elements(&c) == 0);
        // Do not retry anymore.
        break;
    }
    else
    {
        // STREAM EOF. Keep trying.
    }
}

// TEST: invalid states.
// 1) Not a STREAM EOF and no more retries.
// 2) STREAM EOF and all elements consumed.
BOARD_AssertState (!STREAM_EOF(&s) && !retries);
BOARD_AssertState (STREAM_EOF(&s) && CYCLIC_Elements(&c) == 0);

// There are two valid states:
// A) STREAM EOF, run out of retries and not all elements consumed.
// B) not a STREAM EOF and all elements consumed.
