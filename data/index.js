/**
 * Javascript for controlling LED strip through WiFi Server Interface.
 * 
 * @author SteveT10
 * @version 1.0
 */

// Unique event listners:
//Rgb Button
document.getElementById('newRGB').addEventListener('click', submitRgbChange);
//Travel Direction checkbox
document.getElementById('reverseDir').addEventListener('change', toggleDirection);

collapsibleSetUp();
sliderSetUp();

/**
 * Sets up collapsible menu elements.
 */
function collapsibleSetUp() {
  const collaps = document.getElementsByClassName('collapsible')
  for (let i = 0; i < collaps.length; i++) {
    collaps[i].addEventListener('click', function () {
      this.classList.toggle('active')
      const content = this.nextElementSibling
      if (content.style.display === 'block') {
        content.style.display = 'none'
      } else {
        content.style.display = 'block'
      }
    })
  }
}

/**
 * Sets up sliders to read and send values.
 */
function sliderSetUp() {
  const sliders = document.getElementsByClassName('slider');
  const sliderValues = document.getElementsByClassName('sliderValue');
  for (let i = 0; i < sliders.length; i++) {
    sliderValues[i].innerHTML = sliders[i].value
    sliders[i].oninput = function () {
      sliderValues[i].innerHTML = sliders[i].value
      sendUpdate(sliders[i], sliders[i].value)
    }
  }
}

/**
 * Updates LED with new RGB changes to selected sections.
 */
async function submitRgbChange () {
  const newColor = document.getElementById('newColor');
  const startLED = document.getElementById('startLED');
  const endLED = document.getElementById('endLED');
  fetch((`update?color=${newColor.value.slice(1)}&start=${startLED.value}`+
          `&end=${endLED.value}`), {
    method: 'GET'
  }) // .slice(1) is to rid "#",  any # will end the url.
  //console.log(`LEDs ${startLED.value} to ${endLED.value}: ${newColor.value}.`);
}

/**
 * Switches direction that LEDs cycle on LED strip.
 */
async function toggleDirection () {
  if (reverseDir.checked) {
    sendUpdate(reverseDir, 1)
  } else {
    sendUpdate(reverseDir, 0)
  }
}

/**
 * Send a update to LED strips if any sliders, 
 * checkboxes, or single variable settings are changed.
 * 
 * @param {HTMLElement} element is the html element to read.
 * @param {number} stateNum is the state value to set
 */
async function sendUpdate (element, stateNum) {
  // console.log(`Changed ${element.id} to ${stateNum}.`);
  fetch(`/update?output=${element.id}&state=${stateNum}`, {
    method: 'GET'
  })
}