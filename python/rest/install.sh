BUILDTIME=`TZ=CN date -u "+%Y_%m_%d"`
RELEASES_NAME=REST_API_$BUILDTIME.zip

echo "Remove odd files..."
find . -name "__pycache__" -exec rm -rf {} \;

echo "Build install package into $RELEASES_NAME..."
zip -r $RELEASES_NAME comm resources utils app.py start.sh
echo "OK!Upload and test"

