<script type="text/javascript"
    src="http://cdn.mathjax.org/mathjax/latest/MathJax.js?config=TeX-AMS-MML_HTMLorMML">
</script>
<script type="text/x-mathjax-config">
    MathJax.Hub.Config({
    tex2jax: {inlineMath: [['$', '$']]},
    "HTML-CSS": {
        fonts: ["Neo-Euler"],
    },
    messageStyle: "none"
  });
</script>

# Lab 06 Networking

## Introduction

In this lab, we implement a basic network stack in the xv6 operating system kernel, focusing on the Intel E1000 network interface card (NIC) driver. The goal is to enable user processes to send and receive UDP packets over a simulated network. This involved writing a device driver for the E1000 NIC, implementing basic network protocols (ARP, IP, UDP), and providing a socket-like interface for user-space applications.

## Setup

To access the source code and run the programs:

```bash
cd Xv6-RISCV
git checkout cow
```

## NIC Driver

The network interface card (NIC) driver in xv6 is implemented in `kernel/e1000.c`. It interacts with the Intel E1000 network adapter, which is emulated by QEMU. The driver is responsible for initializing the device, transmitting packets, and receiving packets via interrupts.

### Packet Transmission (`e1000_transmit`)

The e1000_tr`ansmit function is called by the network stack to send a packet. It takes a buffer containing the Ethernet frame and its length.

* It acquires the `e1000_lock` to protect access to the TX ring.
* It checks the status of the next TX descriptor (`E1000_TDT`) to ensure it's available (`E1000_TXD_STAT_DD`). If not, it indicates the ring is full and returns an error.
* It frees the buffer associated with the descriptor if one exists (from a previous transmission).
* It stores the new packet buffer pointer in `tx_bufs`.
* It sets up the TX descriptor with the buffer address, length, and command flags (`E1000_TXD_CMD_EOP` and `E1000_TXD_CMD_RS`).
* It updates the `E1000_TDT` register to notify the hardware about the new packet.
* It releases the lock.

```c
int
e1000_transmit(char *buf, int len)
{
  uint64 tx_idx;

  acquire(&e1000_lock);
  tx_idx = regs[E1000_TDT];
  // Check if the TX ring is overflowing.
  if ((tx_ring[tx_idx].status & E1000_TXD_STAT_DD) == 0){
    printf("transmit: TX ring is overflowing\n");
    release(&e1000_lock);
    return -1;
  }

  // Free the previous buffer if it exists.
  if (tx_bufs[tx_idx]){
    kfree(tx_bufs[tx_idx]);
  }
  tx_bufs[tx_idx] = buf;

  // [E1000 3.3] Set up the TX descriptor.
  tx_ring[tx_idx].addr = (uint64)buf;
  tx_ring[tx_idx].length = len;
  tx_ring[tx_idx].cso = 0;
  tx_ring[tx_idx].cmd = E1000_TXD_CMD_EOP | E1000_TXD_CMD_RS;
  tx_ring[tx_idx].status = 0;
  tx_ring[tx_idx].css = 0;
  tx_ring[tx_idx].special = 0;

  // Update the TDT (Transmit Descriptor Tail) register.
  regs[E1000_TDT] = (tx_idx + 1) % TX_RING_SIZE;
  release(&e1000_lock);
  
  return 0;
}
```

### Packet Reception (`e1000_recv`)

Packet reception is interrupt-driven. When the E1000 receives a packet and writes it to a buffer in the RX ring, it generates an interrupt, which calls e1000_intr.

* `e1000_intr` acknowledges the interrupt by writing to `E1000_ICR`.
* It then calls `e1000_recv` to process received packets.
* `e1000_recv` checks the RX ring starting from the descriptor after `E1000_RDT`.
* For each descriptor with the `E1000_RXD_STAT_DD` status bit set (indicating a received packet):
  * It calls `net_rx` to pass the received buffer and length to the network stack.
  * It allocates a new buffer using `kalloc()`.
  * It updates the RX descriptor with the address of the new buffer and clears the status bit.
  * It updates the `E1000_RDT` register to indicate the last descriptor the driver has processed.
  * It moves to the next descriptor in the ring.

```c
static void
e1000_recv(void)
{
  uint64 rx_idx;
  char *buf;

  rx_idx = (regs[E1000_RDT] + 1) % RX_RING_SIZE;
  while (rx_ring[rx_idx].status & E1000_RXD_STAT_DD) {
    // Deliver the packet buffer to the network stack.
    net_rx(rx_bufs[rx_idx], rx_ring[rx_idx].length);

    // Allocate a buffer for the received packet.
    buf = kalloc();
    if(!buf) {
      printf("recv: failed to allocate buffer\n");
      break;
    }
    rx_bufs[rx_idx] = buf;
    rx_ring[rx_idx].addr = (uint64)buf;

    // Clear the status of the RX descriptor.
    rx_ring[rx_idx].status = 0;

    // Update the RDT (Receive Descriptor Tail) register.
    regs[E1000_RDT] = rx_idx;

    // Advance to the next RX descriptor.
    rx_idx = (rx_idx + 1) % RX_RING_SIZE;
  }
}
```

## UDP Packet Receive

The UDP packet reception path involves several functions in `kernel/net.c`, starting from `net_rx` which is called by the NIC driver (`e1000_recv`).

### Network Layer Receive (`net_rx`, `ip_rx`)

* `net_rx` receives the raw packet buffer from the driver. It checks the Ethernet type field to determine the protocol (ARP or IP).
* If it's an IP packet (`ETHTYPE_IP`), it calls `ip_rx`.
* `ip_rx` examines the IP protocol field (`ip_p`) to determine the transport layer protocol.
* If the protocol is UDP (`IPPROTO_UDP`), it calls `udp_rx`. If it's ICMP (`IPPROTO_ICMP`), it calls `icmp_rx`. For other protocols or invalid packets, it frees the buffer.

### UDP Receive Processing (`udp_rx`)

The `udp_rx` function handles incoming UDP packets:

* It extracts the destination port (`dport`) from the UDP header.
* It checks the `bindmap` to see if any socket is bound to this destination port. If not, the packet is dropped by freeing the buffer.
* It iterates through the `sockets` array to find the socket associated with the destination port.
* If no socket is found, the packet is dropped.
* If a socket is found, it acquires the socket's lock.
* It checks if the socket's receive queue (`sock->queue`) is full (sock->count >= RX_QUEUE_SIZE). If full, the packet is dropped.
* If the queue is not full, it adds the packet buffer (`buf`) to the queue, updates the tail index and count.
* It releases the socket's lock.
* It calls `wakeup(sock)` to wake up any process that might be sleeping in `sys_recv` waiting for a packet on this socket.

```c
void
udp_rx(char *buf, int len, struct ip *inip)
{
  uint16 dport;
  struct socket *sock;
  struct udp *inudp;
  int sock_idx;

  inudp = (struct udp *)(inip + 1);
  dport = ntohs(inudp->dport);

  // Check if the destination port is bound.
  acquire(&bindmap_lock);
  if(!(bindmap[dport / sizeof(uint64)] & (1U << (dport % sizeof(uint64))))) {
    release(&bindmap_lock);
    kfree(buf);
    return;
  }
  release(&bindmap_lock);

  // Find the socket for the destination port.
  for(sock_idx = 0; sock_idx < MAX_SOCKETS; sock_idx++) {
    sock = &sockets[sock_idx];
    if(sock->type == IPPROTO_UDP && sock->local_port == dport) {
      break;
    }
  }

  // If no socket found for the port, drop the packet.
  if(sock_idx == MAX_SOCKETS) {
    printf("udp_rx: No socket found for port %d\n", dport);
    kfree(buf);
    return;
  }

  // Check if the receive queue for the socket is full.
  acquire(&sock->lock);
  if(sock->count >= RX_QUEUE_SIZE) {
    release(&sock->lock);
    kfree(buf);
    return;
  }

  // Add the packet to the socket's receive queue.
  sock->queue[sock->tail] = buf;
  sock->tail = (sock->tail + 1) % RX_QUEUE_SIZE;
  sock->count++;

  // Wake up any process waiting on the receive queue.
  release(&sock->lock);
  wakeup(sock);
}
```

### Receiving Data in User Space (`sys_recv`)

The `sys_recv` system call allows a user process to receive a UDP packet:

* It takes the destination port (`dport`), pointers for source IP (`src`) and source port (`sport`), a buffer (`buf`) for the payload, and the maximum length (`maxlen`).
* It finds the corresponding socket based on the `dport`.
* It acquires the socket's lock and enters a `sleep` loop if the queue is empty (`sock->count == 0`), waiting for `udp_rx` to add a packet and call `wakeup`.
* Once a packet is available, it dequeues the packet buffer from the head of the queue, updates the head index and count.
* It releases the socket's lock.
* It parses the packet to extract the source IP and source port.
* It uses `copyout` to copy the source IP, source port, and the UDP payload to the user-provided buffers.
* It calculates the number of bytes copied.
* Crucially, it calls `kfree(packet)` to free the packet buffer that was dequeued from the socket's queue.
* It returns the number of bytes copied.

```c
uint64
sys_recv(void)
{
  int dport, maxlen, sock_idx;
  char *packet;
  uint64 copied_bytes, payload_len, src, sport, buf;
  struct socket *sock;
  struct eth *recv_eth;
  struct ip *recv_ip;
  struct udp *recv_udp;
  struct proc *p = myproc();

  argint(0, &dport);
  argaddr(1, &src);
  argaddr(2, &sport);
  argaddr(3, &buf);
  argint(4, &maxlen);

  // Find the socket for the destination port.
  acquire(&sockets_lock);
  for(sock_idx = 0; sock_idx < MAX_SOCKETS; sock_idx++){
    sock = &sockets[sock_idx];
    if(sock->type == IPPROTO_UDP && sock->local_port == dport){
      break;
    }
  }

  if(sock_idx == MAX_SOCKETS){
    release(&sockets_lock);
    return -1;
  }
  release(&sockets_lock);

  // Sleep until a packet is available.
  acquire(&sock->lock);
  while(sock->count == 0){
    sleep(sock, &sock->lock);
  }

  // Now we have a packet to receive.
  packet = sock->queue[sock->head];
  sock->head = (sock->head + 1) % RX_QUEUE_SIZE;
  sock->count--;
  release(&sock->lock);

  // Parse the packet.
  recv_eth = (struct eth *)packet;
  recv_ip = (struct ip *)(recv_eth + 1);
  recv_udp = (struct udp *)(recv_ip + 1);

  // Copy the source IP and port to user space.
  uint32 src_host = ntohl(recv_ip->ip_src);
  uint16 sport_host = ntohs(recv_udp->sport);

  if(copyout(p->pagetable, src, (char *)&src_host, sizeof(src_host)) < 0)
    panic("recv: copyout src failed");
  if(copyout(p->pagetable, sport, (char *)&sport_host, sizeof(sport_host)) < 0)
    panic("recv: copyout sport failed");

  // Copy the UDP payload to the user buffer.
  if(copyout(p->pagetable, (uint64)buf, (char *)(recv_udp + 1), maxlen) < 0)
    panic("recv: copyout buf failed");

  // Calculate the number of bytes copied.
  payload_len = ntohs(recv_udp->ulen) - sizeof(struct udp);
  copied_bytes = payload_len > maxlen ? maxlen : payload_len;
  kfree(packet); // Free the packet buffer after copying.

  return copied_bytes;
}
```

### Socket Management (`allocsock`, `freesock`)

* `allocsock` finds a free `struct socket` entry in the `sockets` array, initializes its fields (type, port, IP), sets up the receive queue pointers and count, and initializes the socket's spinlock.
* `freesock` finds the socket by port number, marks it as free, and iterates through its receive queue to kfree all queued packet buffers before resetting the queue state.

## Relevant Files

* `kernel/e1000.c`: Implementation of the E1000 network interface card driver.
* `kernel/net.c`: Implementation of the basic network stack, including ARP, IP, UDP, and socket management.
* `kernel/net.h`: Header file defining network structures (Ethernet, IP, UDP, ARP, DNS), socket structure, and function prototypes.
* `kernel/sysproc.c`: Contains the system call implementations for network operations (`sys_bind`, `sys_unbind`, `sys_send`, `sys_recv`).
* `user/nettest.c`: User-level program used for testing the network stack (e.g., ping, UDP tests).

## Key Learning Outcomes

This lab provided hands-on experience in building a basic network stack within an operating system kernel. Key learning outcomes include:

* **Device Driver Interaction:** Understanding how an operating system kernel interacts with hardware devices, specifically a network interface card (NIC), through memory-mapped registers and descriptor rings.
* **Packet Processing Pipeline:** Learning the flow of network packets from the hardware (E1000) through the driver (`e1000.c`) and into the network stack (`net.c`), including interrupt handling for received packets.
* **Network Protocol Implementation:** Implementing basic logic for fundamental network protocols like ARP (Address Resolution Protocol), IP (Internet Protocol), and UDP (User Datagram Protocol), including header parsing and checksum calculation.
* **Socket Abstraction:** Building a simple socket-like abstraction in the kernel to allow user processes to send and receive UDP packets via system calls (`bind`, `unbind`, `send`, `recv`).
* **Kernel Memory Management:** Using `kalloc` and `kfree` to manage memory buffers for network packets and understanding the challenges of buffer ownership and lifetime in a multi-layered network stack.
* **Synchronization:** Applying spinlocks to protect shared data structures like the NIC's descriptor rings, the port binding bitmap, and the socket receive queues from concurrent access by different kernel threads or interrupt handlers.
* **System Call Implementation:** Adding new system calls to expose network functionality to user-space applications.

## Challenges and Solutions

Implementing the networking stack in xv6 presented several challenges:

* **Understanding E1000 Hardware:** Deciphering the E1000 device specification and register layout to correctly initialize the device and manage the transmit and receive rings. 
* **Solution**: Carefully studying the E1000 documentation and the provided `e1000_dev.h` definitions.
* **Correct Ring Buffer Management:** Implementing the logic for adding packets to the TX ring and processing received packets from the RX ring, including updating head/tail pointers and handling descriptor status bits. Ensuring proper synchronization with the hardware. 
* **Solution**: Following the E1000 ring buffer model described in the documentation and using spinlocks to protect shared state.
* **Packet Buffer Ownership and Lifetime:** Managing the allocation and freeing of packet buffers across different layers of the network stack (driver, IP layer, UDP layer, socket queue, system call). Ensuring buffers are freed exactly once to prevent memory leaks or double frees. 
* **Solution**: Carefully tracing the ownership of buffers and implementing `kfree` calls at the appropriate points (e.g., after transmission is complete, when a packet is dropped, after a packet is received by a user process, when a socket is freed).
* **Synchronization Issues:** Protecting shared data structures (like the `e1000_lock`, `bindmap_lock`, `sockets_lock`, and individual `sock->lock`) from race conditions between the interrupt handler and user processes making system calls. 
* **Solution**: Consistently acquiring and releasing the appropriate spinlocks before accessing shared data.
* **Implementing Network Protocols:** Correctly parsing packet headers and implementing the basic logic for ARP, IP, and UDP. This involved understanding network byte order and using functions like `ntohs` and `ntohl`. 
* **Solution**: Referencing network protocol specifications and using the provided byte swap functions.
* **Socket Queue Implementation:** Building a thread-safe queue for received packets for each socket and integrating it with the `sleep`/`wakeup` mechanism to allow user processes to block until data arrives. 
* **Solution**: Implementing a circular buffer (or linked list) with a spinlock and using `sleep` and `wakeup` on the socket structure.
* **Memory Leaks in Socket Queues:** A specific challenge was ensuring that packet buffers queued in a socket's receive queue are freed when the socket is unbound, even if the user process hasn't called `recv` for all packets. Solution: Implementing logic in `freesock` to iterate through the queue and `kfree` any remaining packet buffers.

## Conclusion

Lab 06 provided a comprehensive introduction to network programming within an operating system kernel. We successfully implemented a basic network driver for the emulated E1000 NIC and built a simple network stack capable of handling ARP, IP, and UDP packets. By implementing socket-like functionality, we enabled user processes to perform network communication. This lab reinforced our understanding of device drivers, network protocols, kernel memory management, synchronization, and the process of adding new system calls to an operating system. The challenges encountered highlighted the complexities of managing shared resources and buffer lifetimes in a concurrent kernel environment.
