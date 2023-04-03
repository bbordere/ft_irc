ip=$(hostname -I | awk '{print $1}')

sed -i "$(($(wc -l < ~/.irssi/config)-2))s/.*/    dcc_own_ip = \"$ip\";/" ~/.irssi/config
