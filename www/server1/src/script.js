// console.log("barev");

// function submitForm() {
//     const formData = new FormData(document.getElementById('myForm'));
//     fetch('/submit', {
//         method: 'POST',
//         body: formData
//     })
//     .then(response => response.text())
//     .then(data => {
//         document.getElementById('response').innerText = data;
//     })
//     .catch(error => {
//         console.error('Error:', error);
//     });
// }

// function fileUpload() {
//     const formData = document.getElementById('file');
//     fetch('/uploader.py', {
//         method: 'POST',
//         body: formData.value
//     })
//     .then(response => response.text())
//     .then(data => {
//         console.log('success:');
//     })
//     .catch(error => {
//         console.error('Error:', error);
//     });
// }

console.log("barev");

function submitForm() {
    const formData = new FormData(document.getElementById('myForm'));
    fetch('/submit', {
        method: 'POST',
        body: formData
    })
    .then(response => response.text())
    .then(data => {
        document.getElementById('response').innerText = data;
    })
    .catch(error => {
        console.error('Error:', error);
    });
}

function uploadFile2() {
    // Get the input element
    const fileInput = document.getElementById('fileInput1');

    // Check if a file is selected
    if (fileInput.files.length > 0) {
      // Create a FormData object
      const formData = new FormData();

      // Append the file to the FormData object
      formData.append('file', fileInput.files[0]);

      // Make a POST request using the Fetch API
      fetch('/uploader.py', {
        method: 'POST',
        body: formData
      })
      .then(response => response.json())
      .then(data => {
        console.log('File uploaded successfully:', data);
      })
      .catch(error => {
        console.error('Error uploading file:', error);
      });
    } else {
      console.warn('Please select a file');
    }
  }

function fileUpload() {
    const formData = new FormData(document.getElementById('fileInput'));
    fetch('/upload.py', {
        method: 'POST',
        body: formData
    })
    .then(response => response.text())
    .then(data => {
        console.log('success:');
    })
    .catch(error => {
        console.error('Error:', error);
    });
}