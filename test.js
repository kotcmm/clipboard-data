const clip = require('./')
console.log(clip.getImage().toString("base64"));
clip.setText("111");
console.log(clip.getText());
