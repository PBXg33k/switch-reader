import requests

#url = "https://forums.e-hentai.org/index.php?act=Login&CODE=01"
url = "http://192.168.0.123:5000/?url=https%3A%2F%2Fforums.e-hentai.org%2Findex.php%3Fact%3DLogin%26CODE%3D01"

# User, Pass, Remember Me, Invisible
data = '{"UserName": "colin969", "PassWord": "madmad", "CookieDate": "1", "Privacy": "0"}'

# Do login to get cookies
s = requests.session()
p = s.post(url, data)

print("Headers")
print(p.headers)
print("Cookies")
print(requests.utils.dict_from_cookiejar(s.cookies));
print("Text");
print(p.text)