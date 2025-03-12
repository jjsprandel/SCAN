Certificates may change. 

For github_cert.pem, the first certificate came from running:
openssl s_client -connect github.com:443 -showcerts

Then select the certificate corresponding to:
 2 s:/C=US/ST=New Jersey/L=Jersey City/O=The USERTRUST Network/CN=USERTrust ECC Certification Authority
   i:/C=GB/ST=Greater Manchester/L=Salford/O=Comodo CA Limited/CN=AAA Certificate Services

The next certificate in github_cert.pem came from running:
openssl s_client -connect objects.githubusercontent.com:443 -showcerts

Then select the certificate corresponding to:
 0 s:/CN=*.github.io
   i:/C=GB/ST=Greater Manchester/L=Salford/O=Sectigo Limited/CN=Sectigo RSA Domain Validation Secure Server CA