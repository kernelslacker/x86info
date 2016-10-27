To re-create the man page you'll have to install the whole pandoc suite and invoke the following commands:

```bash
sed -e '15r ../README.md' 'man_template.pdc' | sed -e '15,30d' > '/tmp/pinkybar.pdc'
pandoc -s -S -f markdown -t man /tmp/pinkybar.pdc -o pinkybar.1
```
