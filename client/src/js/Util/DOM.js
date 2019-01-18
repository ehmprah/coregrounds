/**
 * @file
 * Everything related to the DOM
 */

export function buildTabs(screens, active) {
  let tabs = '';
  Object.keys(screens).forEach((screen) => {
    if (screen === active) {
      tabs += `<button class="box active">${screens[screen]}</button>`;
    } else {
      tabs += `<button class="box color-grayscale" onclick="Screens.show('${screen}')">${screens[screen]}</button>`;
    }
  });
  return `<div class="tabs">${tabs}</div>`;
}

export function throttle(fn, threshhold, scope, ...args) {
  // eslint-disable-next-line
  threshhold || ( threshhold = 250 );
  let last;
  let deferTimer;
  return () => {
    const context = scope || this;
    const now = +new Date();
    if (last && now < last + threshhold) {
      // hold on to it
      clearTimeout(deferTimer);
      deferTimer = setTimeout(() => {
        last = now;
        fn.apply(context, args);
      }, threshhold);
    } else {
      last = now;
      fn.apply(context, args);
    }
  };
}

export function removeHTML(element) {
  while (element.firstChild) {
    element.removeChild(element.firstChild);
  }
}

export function adoptHTML(string, element = false) {
  const template = document.createElement('template');
  template.innerHTML = string;
  if (element) {
    element.appendChild(template.content);
  } else {
    document.body.appendChild(template.content);
  }
}

export function submitHandler(id, validate, submit) {
  // Attach click handler to button
  const btn = document.getElementById(id);
  if (btn) {
    btn.addEventListener('click', (e) => {
      e.preventDefault();
      if (validate === false || validate()) {
        btn.classList.add('waiting');
        submit();
      }
    });
  }
}

export function validateEmail(email) {
  // eslint-disable-next-line
  var re = /^(([^<>()\[\]\\.,;:\s@"]+(\.[^<>()\[\]\\.,;:\s@"]+)*)|(".+"))@((\[[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}\])|(([a-zA-Z\-0-9]+\.)+[a-zA-Z]{2,}))$/;
  return re.test(email);
}
