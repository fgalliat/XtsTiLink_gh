clear
echo ""
echo " ~= My RPI 0w =~"
uname -a
echo ""
# top -n 1 -b | head -n 7

# echo -e "\033[05;01;32;44mHello world\033[0m"

# GFX only cls
echo -e "\033(J"

# rectangle
echo -e "\033(10;80;50;40;4r"

# Polyline
echo -e "\033(10;80;30;40;50;80l"

# reversed text
echo -e "\033(10;82;coucou;2t"

#read -n1 var
#echo "You  typed $var"

while read -rsn1 ui; do
    case "$ui" in
    $'\x1b')    # Handle ESC sequence.
        # Flush read. We account for sequences for Fx keys as
        # well. 6 should suffice far more then enough.
        read -rsn1 -t 0.1 tmp
        if [[ "$tmp" == "[" ]]; then
            read -rsn1 -t 0.1 tmp
            case "$tmp" in
            "A") echo "Up\n";;
            "B") echo "Down\n";;
            "C") echo "Right\n";;
            "D") echo "Left\n";;
            esac
        fi
        # Flush "stdin" with 0.1  sec timeout.
        read -rsn5 -t 0.1
        ;;
    # Other one byte (char) cases. Here only quit.
    q) break;;
    esac
done

echo "done."