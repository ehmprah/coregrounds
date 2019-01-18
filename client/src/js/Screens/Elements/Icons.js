/**
 * @file
 * Central icon repository
 */
import about from '../../../img/icons/icon-about.svg';
import add from '../../../img/icons/icon-add.svg';
import bonus from '../../../img/icons/icon-bonus.svg';
import changelog from '../../../img/icons/icon-changelog.svg';
import close from '../../../img/icons/icon-close.svg';
import code from '../../../img/icons/icon-code.svg';
import community from '../../../img/icons/icon-community.svg';
import credits from '../../../img/icons/icon-core-credits.svg';
import currency from '../../../img/icons/icon-currency.svg';
import emote from '../../../img/icons/icon-emote.svg';
import feedback from '../../../img/icons/icon-feedback.svg';
import globalTarget from '../../../img/icons/icon-global-target.svg';
import help from '../../../img/icons/icon-help.svg';
import home from '../../../img/icons/icon-home.svg';
import lock from '../../../img/icons/icon-lock.svg';
import matchHistory from '../../../img/icons/icon-match-history.svg';
import menu from '../../../img/icons/icon-menu.svg';
import modify from '../../../img/icons/icon-modify.svg';
import next from '../../../img/icons/icon-next.svg';
import play from '../../../img/icons/icon-play.svg';
import prev from '../../../img/icons/icon-prev.svg';
import quit from '../../../img/icons/icon-quit.svg';
import scoregrounds from '../../../img/icons/icon-scoregrounds.svg';
import settings from '../../../img/icons/icon-settings.svg';
import spectate from '../../../img/icons/icon-spectate.svg';
import stats from '../../../img/icons/icon-stats.svg';
import store from '../../../img/icons/icon-store.svg';
import support from '../../../img/icons/icon-support.svg';
import swap from '../../../img/icons/icon-swap.svg';
import target from '../../../img/icons/icon-target.svg';
import traininggrounds from '../../../img/icons/icon-training-grounds.svg';
import unlock from '../../../img/icons/icon-unlock.svg';
import wait from '../../../img/icons/icon-wait.svg';

const icons = {
  about,
  add,
  bonus,
  changelog,
  close,
  code,
  community,
  credits,
  currency,
  emote,
  feedback,
  globalTarget,
  help,
  home,
  lock,
  matchHistory,
  menu,
  modify,
  next,
  play,
  prev,
  quit,
  scoregrounds,
  settings,
  spectate,
  stats,
  store,
  support,
  swap,
  target,
  traininggrounds,
  unlock,
  wait,
};

export default {
  get(key) {
    return icons[key];
  },
};

CanvasRenderingContext2D.prototype.drawIcon = function icon(key, color, size = 80, base = 128) {
  const img = new Image();
  const offset = (base - size) / 2;
  img.onload = () => {
    this.drawImage(img, offset, offset, size, size);
  };

  if (icons[key] === undefined) {
    throw new Error(`Icon with key ${key} not found`);
  }
  const svg = icons[key].replace(/fill="#000000"/gi, `fill="${color}"`);
  img.src = `data:image/svg+xml;base64,${window.btoa(svg)}`;
};
