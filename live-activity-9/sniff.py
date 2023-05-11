from scapy.all import *

print("SNIFFING PACKETS...")

def print_pkt(pkt):
   print("Source IP:", pkt[IP].src)
   print("Destination IP:", pkt[IP].dst)
   print("Protocol:", pkt[IP].proto)
   print("\n")

pkt = sniff(filter = 'ether src 00:11:22:33:44:55 and ether dst 55:44:33:22:11:00 and dst host 192.168.0.2', timeout = 10, prn = print_pkt)
# print_pkt(pkt)