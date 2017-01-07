#!/bin/sh

if [ "x$1" = "x" ]; then
 echo "Format: $0 tag [x]"
 echo " An x means that it'll read options from the terminal."
 exit 1
fi

the_tag="james-$1"
do_skel=true

echo -n " Is that the full tag? [N] "
read a;
case x$a in
 xy|xye|xyes)
  the_tag="$1"
 ;;

 x|xn|xno)
 ;;

 *)
  echo " Not a valid answer."
  exit 1
esac

echo -n " Do pre skels? [Y] "
read a;
case x$a in
 x|xy|xye|xyes)
 ;;

 xn|xno)
 do_skel="false"
 ;;

 *)
  echo " Not a valid answer."
  exit 1
esac

thepwd="`pwd`"

echo "Exporting code..."
cvs export -r $the_tag talker &> /dev/null

echo "Fixing code..."
if [ ! -d talker/src/scripts ]; then
 echo "can't find dir properly... using tag $the_tag"
 exit 1;
fi

cd talker/src/scripts && autoconf

cd $thepwd/talker/src/lib/socket_poll && ./pre_configure
cd $thepwd/talker/src/lib/timer_q && ./pre_configure

if [ "$do_skel" = "true" ]; then

 echo " Doing skel changes."

cd $thepwd/talker/skel/pre_copy

find . -print0 | xargs -0 touch
find . -print0 | cpio -0 -p --make-directories ../

cd $thepwd/talker
cat $thepwd/talker/skel/pre_rm | xargs rm

fi

cd $thepwd/talker/skel/dist_copy

find . -type f -print > ../dist_copy_index

cd $thepwd

mv talker twocan-$the_tag

echo "Taring code..."

tar -cf twocan-$the_tag.tar twocan-$the_tag

echo "Ziping code..."

bzip2 -9 twocan-$the_tag.tar

