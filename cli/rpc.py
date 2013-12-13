import json

try :
    import urllib.request
    urlopen = urllib.request.urlopen
    error = urllib.error.URLError
except ImportError :
    import urllib
    urlopen = urllib.urlopen
    error = IOError

def call(method, params) :
    # prepare request
    req = {
        "jsonrpc" : "2.0",
        "method" : method,
        "id" : 1,
        "params" : params
    }

    try :
        f = urlopen('http://localhost:8080', json.dumps(req).encode('utf-8'))
    except error :
        return None

    data = json.loads(f.read().decode('utf-8'))
    f.close()

    if "result" not in data :
        return None

    return data["result"]
