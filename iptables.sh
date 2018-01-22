#!/bin/sh

# iptables 初期化
iptables -F INPUT
iptables -F OUTPUT
iptables -F FORWARD

# 受信を破棄 / 送信を許可 / 通過を拒否
iptables -P INPUT DROP
iptables -P OUTPUT ACCEPT
iptables -P FORWARD DROP

# 自ホストからのアクセスをすべて許可
iptables -A INPUT -i lo -j ACCEPT
iptables -A OUTPUT -o lo -j ACCEPT

# 内部から行ったアクセスに対する外部からの返答アクセスを許可
iptables -A INPUT -m state --state ESTABLISHED,RELATED -j ACCEPT

# ping(icmp)許可
iptables -I INPUT -p icmp --icmp-type 0 -j ACCEPT
iptables -I INPUT -p icmp --icmp-type 8 -j ACCEPT

# ssh許可
iptables -A INPUT -p tcp --dport 22 -j ACCEPT

# 4567 を許可
iptables -A INPUT -p tcp --dport 4567 -j ACCEPT


# 上記のルールにマッチしなかったアクセスはログを記録して破棄
iptables -A INPUT -m limit --limit 1/s -j LOG --log-prefix '[IPTABLES INPUT] : '
iptables -A INPUT -j DROP
iptables -A FORWARD -m limit --limit 1/s -j LOG --log-prefix '[IPTABLES FORWARD] : '
iptables -A FORWARD -j DROP
