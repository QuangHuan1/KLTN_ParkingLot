const auth = JSON.parse(localStorage.getItem("auth"));
if (!auth?.username && window.location.href.split("/")[3] !== 'login' && window.location.href.split("/")[3] !== 'phone') {
    console.log(window.location.href.split("/"))
    location.replace("/login");
}
