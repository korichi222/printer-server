package main

import (
    "fmt"
    "io"
    "os"
    "os/exec"
    "log"
    "net/http"
)

func print(w http.ResponseWriter, r *http.Request) {
    r.ParseMultipartForm(32 << 20)

    file, _, err := r.FormFile("file")

    defer file.Close()
    f, err := os.Create("filelll")
    if err != nil {
        log.Fatal(err)
    }
    pageCount := r.FormValue("pageCount")
    divs := r.FormValue("divs")
    fmt.Fprintf(w, "hello\n")
    _, err = io.Copy(f, file)
    f.Close()
    printExe := exec.Command("./win32.exe","filelll",pageCount,divs)
    printExe.Start()
    printExe.Wait()
    os.Remove("filelll")
}

func headers(w http.ResponseWriter, req *http.Request) {
    for name, headers := range req.Header {
        for _, h := range headers {
            fmt.Fprintf(w, "%v: %v\n", name, h)
        }
    }
}

func main() {
    http.HandleFunc("/print", print)
http.HandleFunc("/", func(w http.ResponseWriter, r *http.Request) {
    http.ServeFile(w, r, "index.html")
})
    http.ListenAndServe(":8090", nil)
}